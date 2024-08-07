/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __DUMMY_BACKEND_HPP
#define __DUMMY_BACKEND_HPP

#include <kage/Backend.hpp>

using json = nlohmann::json;

/**
 * Dummy implementation of an kage Backend.
 */
class DummyProxy : public kage::Backend {

    thallium::engine m_engine;
    json             m_config;

    public:

    /**
     * @brief Constructor.
     */
    DummyProxy(thallium::engine engine, const json& config);

    /**
     * @brief Move-constructor.
     */
    DummyProxy(DummyProxy&&) = default;

    /**
     * @brief Copy-constructor.
     */
    DummyProxy(const DummyProxy&) = default;

    /**
     * @brief Move-assignment operator.
     */
    DummyProxy& operator=(DummyProxy&&) = default;

    /**
     * @brief Copy-assignment operator.
     */
    DummyProxy& operator=(const DummyProxy&) = default;

    /**
     * @brief Destructor.
     */
    virtual ~DummyProxy() = default;

    /**
     * @brief Get the proxy's configuration as a JSON-formatted string.
     */
    std::string getConfig() const override;

    /**
     * @brief Prints Hello World.
     */
    void sayHello() override;

    /**
     * @brief Compute the sum of two integers.
     *
     * @param x first integer
     * @param y second integer
     *
     * @return a Result containing the result.
     */
    kage::Result<int32_t> computeSum(int32_t x, int32_t y) override;

    /**
     * @brief Destroys the underlying proxy.
     *
     * @return a Result<bool> instance indicating
     * whether the database was successfully destroyed.
     */
    kage::Result<bool> destroy() override;

    /**
     * @brief Static factory function used by the ProxyFactory to
     * create a DummyProxy.
     *
     * @param engine Thallium engine
     * @param config JSON configuration for the proxy
     *
     * @return a unique_ptr to a proxy
     */
    static std::unique_ptr<kage::Backend> create(const thallium::engine& engine, const json& config);

    /**
     * @brief Static factory function used by the ProxyFactory to
     * open a DummyProxy.
     *
     * @param engine Thallium engine
     * @param config JSON configuration for the proxy
     *
     * @return a unique_ptr to a proxy
     */
    static std::unique_ptr<kage::Backend> open(const thallium::engine& engine, const json& config);
};

#endif
