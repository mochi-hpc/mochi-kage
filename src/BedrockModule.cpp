/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "kage/Provider.hpp"
#include "kage/ProviderHandle.hpp"

#include <bedrock/AbstractComponent.hpp>

#include <nlohmann/json.hpp>

namespace tl = thallium;
using json = nlohmann::json;

class KageComponent : public bedrock::AbstractComponent {

    std::unique_ptr<kage::Provider> m_provider;

    public:

    KageComponent(const tl::engine& engine,
                  uint16_t provider_id,
                  const char* identity,
                  const std::string& config,
                  tl::provider_handle target,
                  const tl::pool& rpc_pool,
                  const tl::pool& proxy_pool)
    : m_provider{std::make_unique<kage::Provider>(
        engine, provider_id, identity, config, target, rpc_pool, proxy_pool)}
    {}

    void* getHandle() override {
        return static_cast<void*>(m_provider.get());
    }

    std::string getConfig() override {
        return m_provider->getConfig();
    }

    static std::shared_ptr<bedrock::AbstractComponent>
        Register(const bedrock::ComponentArgs& args) {
            tl::pool rpc_pool, proxy_pool;
            auto config = json::parse(args.config);
            std::string identity = config["identity"];
            auto it = args.dependencies.find("rpc_pool");
            if(it != args.dependencies.end() && !it->second.empty()) {
                rpc_pool = it->second[0]->getHandle<tl::pool>();
            }
            it = args.dependencies.find("proxy_pool");
            if(it != args.dependencies.end() && !it->second.empty()) {
                proxy_pool = it->second[0]->getHandle<tl::pool>();
            }
            tl::provider_handle target;
            it = args.dependencies.find("target");
            if(it != args.dependencies.end() && !it->second.empty()) {
                target = it->second[0]->getHandle<tl::provider_handle>();
            }
            return std::make_shared<KageComponent>(
                args.engine, args.provider_id, identity.c_str(),
                args.config, target, rpc_pool, proxy_pool);
        }

    static std::vector<bedrock::Dependency>
        GetDependencies(const bedrock::ComponentArgs& args) {
            auto config = json::parse(args.config);
            if(config.is_object())
                throw bedrock::Exception{"Configuration for kage provider should be an object"};
            if(!config.contains("identity") || !config["identity"].is_string())
                throw bedrock::Exception{
                    "Configuration for kage provider should contain an "
                        "\"identity\" field of type string"};
            std::string identity = config["identity"];
            std::vector<bedrock::Dependency> dependencies{
                bedrock::Dependency{
                    /* name */ "rpc_pool",
                    /* type */ "pool",
                    /* is_required */ false,
                    /* is_array */ false,
                    /* is_updatable */ false
                },
                bedrock::Dependency{
                    /* name */ "proxy_pool",
                    /* type */ "pool",
                    /* is_required */ false,
                    /* is_array */ false,
                    /* is_updatable */ false
                },
                bedrock::Dependency{
                    /* name */ "target",
                    /* type */ identity,
                    /* is_required */ false,
                    /* is_array */ false,
                    /* is_updatable */ false
                }
            };
            return dependencies;
        }
};

BEDROCK_REGISTER_COMPONENT_TYPE(kage, KageComponent)
