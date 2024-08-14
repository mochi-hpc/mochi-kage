/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __MARGO_BACKEND_HPP
#define __MARGO_BACKEND_HPP

#include <zmq.hpp>
#include <kage/Backend.hpp>

using json = nlohmann::json;

/**
 * Margo implementation of an kage Backend.
 */
class MargoProxy : public kage::Backend {

    json                       m_config;
    kage::InputProxy           m_input_proxy;
    thallium::engine           m_internal_engine;
    thallium::endpoint         m_remote_endpoint;
    thallium::remote_procedure m_rpc;

    public:

    /**
     * @brief Constructor.
     */
    MargoProxy(json&& config,
               thallium::engine internal_engine,
               thallium::endpoint remote_endpoint);

    /**
     * @brief Move-constructor.
     */
    MargoProxy(MargoProxy&&) = delete;

    /**
     * @brief Copy-constructor.
     */
    MargoProxy(const MargoProxy&) = delete;

    /**
     * @brief Move-assignment operator.
     */
    MargoProxy& operator=(MargoProxy&&) = delete;

    /**
     * @brief Copy-assignment operator.
     */
    MargoProxy& operator=(const MargoProxy&) = delete;

    /**
     * @brief Destructor.
     */
    virtual ~MargoProxy() = default;

    /**
     * @brief Get the proxy's configuration as a JSON-formatted string.
     */
    std::string getConfig() const override;

    /**
     * @see Backend::forward
     */
    kage::Result<bool> forwardOutput(hg_id_t rpc_id, const char* input, size_t input_size,
                                     const std::function<void(const char*, size_t)>& output_cb) override;

    /**
     * @see Backend::setInputProxy
     */
    void setInputProxy(kage::InputProxy proxy) override;

    /**
     * @brief Destroys the underlying proxy.
     *
     * @return a Result<bool> instance indicating
     * whether the database was successfully destroyed.
     */
    kage::Result<bool> destroy() override;

    /**
     * @brief Static factory function used by the ProxyFactory to
     * create a MargoProxy.
     *
     * @param engine Thallium engine
     * @param config JSON configuration for the proxy
     * @param pool Optional pool in which to submit work.
     *
     * @return a unique_ptr to a proxy
     */
    static std::unique_ptr<kage::Backend> create(
            const thallium::engine& engine,
            const json& config,
            const thallium::pool& pool);
};

#endif
