/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __KAGE_PROVIDER_IMPL_H
#define __KAGE_PROVIDER_IMPL_H

#include "kage/Backend.hpp"
#include "Serialization.hpp"

#include <thallium.hpp>
#include <thallium/serialization/stl/string.hpp>
#include <thallium/serialization/stl/vector.hpp>

#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <memory>
#include <tuple>

namespace kage {

using namespace std::string_literals;
namespace tl = thallium;
using nlohmann::json;
using nlohmann::json_schema::json_validator;

class ProviderImpl : public tl::provider<ProviderImpl>, std::enable_shared_from_this<ProviderImpl> {

    auto id() const { return get_provider_id(); } // for convenience

    using json = nlohmann::json;

    #define DEF_LOGGING_FUNCTION(__name__)                         \
    template<typename ... Args>                                    \
    void __name__(Args&&... args) {                                \
        auto msg = fmt::format(std::forward<Args>(args)...);       \
        spdlog::__name__("[kage:{}] {}", get_provider_id(), msg); \
    }

    struct RPC {

        tl::remote_procedure proc;
        std::string          name;

        RPC(tl::remote_procedure&& rpc, std::string n)
        : proc{std::move(rpc)}
        , name{n} {}

        RPC(RPC&&) = default;
    };

    DEF_LOGGING_FUNCTION(trace)
    DEF_LOGGING_FUNCTION(debug)
    DEF_LOGGING_FUNCTION(info)
    DEF_LOGGING_FUNCTION(warn)
    DEF_LOGGING_FUNCTION(error)
    DEF_LOGGING_FUNCTION(critical)

    #undef DEF_LOGGING_FUNCTION

    public:

    tl::engine           m_engine;
    tl::pool             m_rpc_pool;
    tl::pool             m_proxy_pool;
    tl::provider_handle  m_target;
    bool                 m_is_input;
    bool                 m_is_output;
    // Exported RPCs
    std::unordered_map<hg_id_t, RPC> m_rpcs;
    // Backend
    std::shared_ptr<Backend> m_backend;

    ProviderImpl(const tl::engine& engine,
                 uint16_t provider_id,
                 const std::string& config,
                 const tl::provider_handle& target,
                 const tl::pool& rpc_pool,
                 const tl::pool& proxy_pool)
    : tl::provider<ProviderImpl>(engine, provider_id, "kage")
    , m_engine{engine}
    , m_rpc_pool{rpc_pool.is_null() ? m_engine.get_handler_pool() : rpc_pool}
    , m_proxy_pool{proxy_pool.is_null() ? m_engine.get_handler_pool() : proxy_pool}
    , m_target{target}
    {
        static const json schema = R"(
        {
            "type": "object",
            "properties": {
                "direction": {
                    "type": "string",
                    "enum": ["in", "out", "inout"]
                },
                "proxy": {
                    "type": "object",
                    "properties": {
                        "type": {"type": "string"},
                        "config": {"type": "object"}
                    },
                    "required": ["type"]
                },
                "exported_rpcs": {
                    "type": "array",
                    "items": { "type": "string", "minLength": 1 }
                }
            },
            "required": ["proxy", "direction", "exported_rpcs"]
        }
        )"_json;
        json_validator validator;
        validator.set_root_schema(schema);

        trace("Registered provider with id {}", get_provider_id());
        json json_config;
        try {
            json_config = json::parse(config);
        } catch(json::parse_error& e) {
            error("Could not parse provider configuration: {}", e.what());
            throw Exception{"Could not parse provider configuration: {}", e.what()};
        }

        try {
            validator.validate(json_config);
        } catch(const std::exception& ex) {
            error("Error(s) while validating JSON config for warabi provider: {}", ex.what());
            throw Exception("Invalid JSON configuration (see error logs for information)");
        }

        // Get direction
        m_is_input = json_config["direction"] == "in" || json_config["direction"] == "inout";
        m_is_output = json_config["direction"] == "out" || json_config["direction"] == "inout";

        if(m_is_input && target.is_null()) {
            throw Exception("Input proxy needs a provider to redirect input to");
        }

        // Export RPCs
        auto& rpcs = json_config["exported_rpcs"];
        if(m_is_output) {
            for(auto& name : rpcs) {
                auto rpc = RPC{
                    m_is_output ?
                        define(name, &ProviderImpl::forwardRPCtoOutput, m_rpc_pool)
                        :  get_engine().define(name),
                    name};
                m_rpcs.insert(std::make_pair(rpc.proc.id(), std::move(rpc)));
            }
        }

        // Create backend
        auto& proxy = json_config["proxy"];
        auto& proxy_type = proxy["type"].get_ref<const std::string&>();
        auto proxy_config = proxy.contains("config") ? proxy["config"] : json::object();
        auto result = createProxy(proxy_type, proxy_config);
        result.check();
    }

    ~ProviderImpl() {
        trace("Deregistering provider");
        if(m_is_output) {
            for(auto& p : m_rpcs) {
                p.second.proc.deregister();
            }
        }
        m_rpcs.clear();
        if(m_backend) {
            m_backend->destroy();
        }
    }

    std::string getConfig() const {
        auto config = json::object();
        if(m_backend) {
            config["proxy"] = json::object();
            auto proxy_config = json::object();
            proxy_config["type"] = m_backend->name();
            proxy_config["config"] = json::parse(m_backend->getConfig());
            config["proxy"] = std::move(proxy_config);
        }
        config["exported_rpcs"] = json::array();
        auto& rpcs = config["exported_rpcs"];
        for(auto& p : m_rpcs) {
            rpcs.push_back(p.first);
        }
        return config.dump();
    }

    Result<bool> createProxy(const std::string& proxy_type,
                             const json& proxy_config) {

        Result<bool> result;

        try {
            m_backend = ProxyFactory::createProxy(
                proxy_type, get_engine(), proxy_config, m_proxy_pool);
        } catch(const std::exception& ex) {
            result.success() = false;
            result.error() = ex.what();
            error("Error when creating proxy of type {}: {}",
                  proxy_type, result.error());
            return result;
        }

        if(not m_backend) {
            result.success() = false;
            result.error() = "Unknown proxy type "s + proxy_type;
            error("Unknown proxy type {}", proxy_type);
            return result;
        }

        trace("Successfully created proxy of type {}", proxy_type);
        return result;
    }

    void forwardRPCtoOutput(const tl::request& req) {
        auto rpc_id = HG_Get_info(req.native_handle())->id;
        Deserializer deserializer{
            [this, rpc_id, &req](const char* input, size_t input_size) {
                auto send_response = [&req](const char * output, size_t output_size) {
                    Serializer serializer{output, output_size};
                    req.respond(serializer);
                };
                m_backend->forwardOutput(rpc_id, input, input_size, send_response);
            }
        };
        req.get_input().unpack(deserializer);
    }

    Result<bool> forwardRPCtoInput(
            hg_id_t rpc_id, const char* input, size_t input_size,
            const std::function<void(const char*, size_t)>& output_cb) {
        Result<bool> result;
        auto rpc_it = m_rpcs.find(rpc_id);
        if(rpc_it == m_rpcs.end()) {
            result.success() = false;
            result.error() = "Provider received unknow RPC id";
            return result;
        }
        auto& rpc = rpc_it->second;

        Serializer serializer{input, input_size};
        auto output = rpc.proc.on(m_target)(serializer);

        Deserializer deserializer{output_cb};
        output.unpack(deserializer);

        return result;
    }

};

}

#endif
