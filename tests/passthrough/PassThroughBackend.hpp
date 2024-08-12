/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __ECHO_BACKEND_HPP
#define __ECHO_BACKEND_HPP

#include <kage/Backend.hpp>

using json = nlohmann::json;

/**
 * PassThrough implementation of an kage Backend.
 */
class PassThroughProxy : public kage::Backend {

    thallium::engine m_engine;
    json             m_config;
    kage::InputProxy m_input_proxy;

    public:

    /**
     * @brief Constructor.
     */
    PassThroughProxy(thallium::engine engine, const json& config);

    /**
     * @brief Move-constructor.
     */
    PassThroughProxy(PassThroughProxy&&) = default;

    /**
     * @brief Copy-constructor.
     */
    PassThroughProxy(const PassThroughProxy&) = default;

    /**
     * @brief Move-assignment operator.
     */
    PassThroughProxy& operator=(PassThroughProxy&&) = default;

    /**
     * @brief Copy-assignment operator.
     */
    PassThroughProxy& operator=(const PassThroughProxy&) = default;

    /**
     * @brief Destructor.
     */
    virtual ~PassThroughProxy() = default;

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
     * create a PassThroughProxy.
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
