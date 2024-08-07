/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __KAGE_PROVIDER_IMPL_H
#define __KAGE_PROVIDER_IMPL_H

#include "kage/Backend.hpp"

#include <thallium.hpp>
#include <thallium/serialization/stl/string.hpp>
#include <thallium/serialization/stl/vector.hpp>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <tuple>

namespace kage {

using namespace std::string_literals;
namespace tl = thallium;

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
    tl::pool             m_pool;
    // Client RPC
    tl::auto_remote_procedure m_compute_sum;
    // FIXME: other RPCs go here ...
    // Backends
    std::shared_ptr<Backend> m_backend;

    ProviderImpl(const tl::engine& engine, uint16_t provider_id, const std::string& config, const tl::pool& pool)
    : tl::provider<ProviderImpl>(engine, provider_id, "kage")
    , m_engine(engine)
    , m_pool(pool)
    , m_compute_sum(define("kage_compute_sum",  &ProviderImpl::computeSumRPC, pool))
    {
        trace("Registered provider with id {}", get_provider_id());
        json json_config;
        try {
            json_config = json::parse(config);
        } catch(json::parse_error& e) {
            error("Could not parse provider configuration: {}", e.what());
            return;
        }
        if(!json_config.is_object()) return;
        if(!json_config.contains("proxy")) return;
        auto& proxy = json_config["proxy"];
        if(!proxy.is_object()) return;
        if(proxy.contains("type") && proxy["type"].is_string()) {
            auto& proxy_type = proxy["type"].get_ref<const std::string&>();
            auto proxy_config = proxy.contains("config") ? proxy["config"] : json::object();
            auto result = createProxy(proxy_type, proxy_config);
            result.check();
        }
    }

    ~ProviderImpl() {
        trace("Deregistering provider");
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
        return config.dump();
    }

    Result<bool> createProxy(const std::string& proxy_type,
                                const json& proxy_config) {

        Result<bool> result;

        try {
            m_backend = ProxyFactory::createProxy(proxy_type, get_engine(), proxy_config);
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

    void computeSumRPC(const tl::request& req,
                       int32_t x, int32_t y) {
        trace("Received computeSum request");
        Result<int32_t> result;
        tl::auto_respond<decltype(result)> response{req, result};
        if(!m_backend) {
            result.success() = false;
            result.error() = "Provider has no proxy attached";
        } else {
            result = m_backend->computeSum(x, y);
        }
        trace("Successfully executed computeSum");
    }

};

}

#endif
