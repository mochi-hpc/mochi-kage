/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __KAGE_PROVIDER_HPP
#define __KAGE_PROVIDER_HPP

#include <thallium.hpp>
#include <memory>

namespace kage {

namespace tl = thallium;

class ProviderImpl;

/**
 * @brief A Provider is an object that can receive RPCs
 * and dispatch them to specific proxys.
 */
class Provider {

    public:

    /**
     * @brief Constructor.
     *
     * @param engine Thallium engine to use to receive RPCs.
     * @param provider_id Provider id.
     * @param config JSON-formatted configuration.
     * @param target Target of input RPCs, if input provider.
     * @param rpc_pool Argobots pool to use to handle RPCs.
     * @param proxy_pool Argobots pool to pass to the proxy.
     */
    Provider(const tl::engine& engine,
             uint16_t provider_id,
             const std::string& config,
             const tl::provider_handle& target = tl::provider_handle{},
             const tl::pool& rpc_pool = tl::pool(),
             const tl::pool& proxy_pool = tl::pool());

    /**
     * @brief Copy-constructor is deleted.
     */
    Provider(const Provider&) = delete;

    /**
     * @brief Move-constructor.
     */
    Provider(Provider&&);

    /**
     * @brief Copy-assignment operator is deleted.
     */
    Provider& operator=(const Provider&) = delete;

    /**
     * @brief Move-assignment operator is deleted.
     */
    Provider& operator=(Provider&&) = delete;

    /**
     * @brief Destructor.
     */
    ~Provider();

    /**
     * @brief Return a JSON-formatted configuration of the provider.
     *
     * @return JSON formatted string.
     */
    std::string getConfig() const;

    /**
     * @brief Checks whether the Provider instance is valid.
     */
    operator bool() const;

    private:

    std::shared_ptr<ProviderImpl> self;
};

}

#endif
