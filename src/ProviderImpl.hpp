/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __KAGE_PROVIDER_IMPL_H
#define __KAGE_PROVIDER_IMPL_H

#include "kage/Backend.hpp"
#include "kage/InputOutputManager.hpp"

#include <thallium.hpp>
#include <thallium/serialization/stl/string.hpp>
#include <thallium/serialization/stl/vector.hpp>

#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <tuple>

namespace kage {

using namespace std::string_literals;
namespace tl = thallium;
using nlohmann::json;
using nlohmann::json_schema::json_validator;

class ProviderImpl : public tl::provider<ProviderImpl> {

    auto id() const { return get_provider_id(); } // for convenience

    using json = nlohmann::json;

    #define DEF_LOGGING_FUNCTION(__name__)                         \
    template<typename ... Args>                                    \
    void __name__(Args&&... args) {                                \
        auto msg = fmt::format(std::forward<Args>(args)...);       \
        spdlog::__name__("[kage:{}] {}", get_provider_id(), msg); \
    }

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
    bool                 m_is_input;
    bool                 m_is_output;
    // Exported RPCs
    std::unordered_map<std::string, tl::remote_procedure> m_rpcs;
    // Backend
    std::shared_ptr<Backend> m_backend;

    ProviderImpl(const tl::engine& engine,
                 uint16_t provider_id,
                 const std::string& config,
                 const tl::provider_handle& target,
                 const tl::pool& rpc_pool,
                 const tl::pool& proxy_pool)
    : tl::provider<ProviderImpl>(engine, provider_id, "kage")
    , m_engine(engine)
    , m_rpc_pool(rpc_pool.is_null() ? m_engine.get_handler_pool() : rpc_pool)
    , m_proxy_pool(proxy_pool.is_null() ? m_engine.get_handler_pool() : proxy_pool)
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
                m_rpcs.insert(std::make_pair(name, define(name, &ProviderImpl::forwardRPCtoOutput, m_rpc_pool)));
            }
        } else {
            for(auto& name : rpcs) {
                m_rpcs.insert(std::make_pair(name, get_engine().define(name)));
            }
        }

        // Create backend
        auto& proxy = json_config["proxy"];
        auto& proxy_type = proxy["type"].get_ref<const std::string&>();
        auto proxy_config = proxy.contains("config") ? proxy["config"] : json::object();
        auto result = createProxy(proxy_type, proxy_config, target);
        result.check();
    }

    ~ProviderImpl() {
        trace("Deregistering provider");
        if(m_is_output) {
            for(auto& p : m_rpcs) {
                p.second.deregister();
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
                             const json& proxy_config,
                             const tl::provider_handle& target) {

        Result<bool> result;

        try {
            m_backend = ProxyFactory::createProxy(
                proxy_type, get_engine(), proxy_config, target, m_proxy_pool);
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
        InputOutputManager inout{*m_backend, req};
        req.get_input().unpack(inout);
    }

};

}

#endif
