/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "kage/Provider.hpp"
#include "kage/ProviderHandle.hpp"
#include <bedrock/AbstractServiceFactory.hpp>

namespace tl = thallium;

class KageFactory : public bedrock::AbstractServiceFactory {

    public:

    KageFactory() {}

    void *registerProvider(const bedrock::FactoryArgs &args) override {
        auto provider = new kage::Provider(args.mid, args.provider_id, "kage",
                args.config, tl::provider_handle{}, tl::pool(args.pool));
        return static_cast<void *>(provider);
    }

    void deregisterProvider(void *p) override {
        auto provider = static_cast<kage::Provider *>(p);
        delete provider;
    }

    std::string getProviderConfig(void *p) override {
        auto provider = static_cast<kage::Provider *>(p);
        return provider->getConfig();
    }

    void *initClient(const bedrock::FactoryArgs& args) override {
        return static_cast<void*>(new tl::engine{args.engine});
    }

    void finalizeClient(void *client) override {
        auto engine = static_cast<tl::engine*>(client);
        delete engine;
    }

    std::string getClientConfig(void* c) override {
        (void)c;
        return "{}";
    }

    void *createProviderHandle(void *c, hg_addr_t address,
            uint16_t provider_id) override {
        auto engine = static_cast<tl::engine*>(c);
        auto ph = new kage::ProviderHandle(
                *engine,
                address,
                provider_id,
                false);
        return static_cast<void *>(ph);
    }

    void destroyProviderHandle(void *providerHandle) override {
        auto ph = static_cast<kage::ProviderHandle *>(providerHandle);
        delete ph;
    }

    const std::vector<bedrock::Dependency> &getProviderDependencies() override {
        static const std::vector<bedrock::Dependency> no_dependency;
        return no_dependency;
    }

    const std::vector<bedrock::Dependency> &getClientDependencies() override {
        static const std::vector<bedrock::Dependency> no_dependency;
        return no_dependency;
    }
};

BEDROCK_REGISTER_MODULE_FACTORY(kage, KageFactory)
