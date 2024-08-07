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
 * Echo implementation of an kage Backend.
 */
class EchoProxy : public kage::Backend {

    thallium::engine m_engine;
    json             m_config;

    public:

    /**
     * @brief Constructor.
     */
    EchoProxy(thallium::engine engine, const json& config);

    /**
     * @brief Move-constructor.
     */
    EchoProxy(EchoProxy&&) = default;

    /**
     * @brief Copy-constructor.
     */
    EchoProxy(const EchoProxy&) = default;

    /**
     * @brief Move-assignment operator.
     */
    EchoProxy& operator=(EchoProxy&&) = default;

    /**
     * @brief Copy-assignment operator.
     */
    EchoProxy& operator=(const EchoProxy&) = default;

    /**
     * @brief Destructor.
     */
    virtual ~EchoProxy() = default;

    /**
     * @brief Get the proxy's configuration as a JSON-formatted string.
     */
    std::string getConfig() const override;

    /**
     * @brief Compute the sum of two integers.
     *
     * @param x first integer
     * @param y second integer
     *
     * @return a Result containing the result.
     */
    kage::Result<bool> forward(const char* input, size_t input_size,
                               const std::function<void(const char*, size_t)>& output_cb) override;

    /**
     * @brief Destroys the underlying proxy.
     *
     * @return a Result<bool> instance indicating
     * whether the database was successfully destroyed.
     */
    kage::Result<bool> destroy() override;

    /**
     * @brief Static factory function used by the ProxyFactory to
     * create a EchoProxy.
     *
     * @param engine Thallium engine
     * @param config JSON configuration for the proxy
     *
     * @return a unique_ptr to a proxy
     */
    static std::unique_ptr<kage::Backend> create(const thallium::engine& engine, const json& config);

    /**
     * @brief Static factory function used by the ProxyFactory to
     * open a EchoProxy.
     *
     * @param engine Thallium engine
     * @param config JSON configuration for the proxy
     *
     * @return a unique_ptr to a proxy
     */
    static std::unique_ptr<kage::Backend> open(const thallium::engine& engine, const json& config);
};

#endif
