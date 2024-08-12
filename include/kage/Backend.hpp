/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __KAGE_BACKEND_HPP
#define __KAGE_BACKEND_HPP

#include <kage/Result.hpp>
#include <kage/InputProxy.hpp>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>
#include <thallium.hpp>

/**
 * @brief Helper class to register backend types into the backend factory.
 */
template<typename BackendType>
class __KageBackendRegistration;

namespace kage {

/**
 * @brief Interface for proxy backends. To build a new backend,
 * implement a class MyBackend that inherits from Backend, and put
 * KAGE_REGISTER_BACKEND(mybackend, MyBackend); in a cpp file
 * that includes your backend class' header file.
 *
 * Your backend class should also have a static functions to create a proxy:
 *
 * std::unique_ptr<Backend> create(const json& config)
 */
class Backend {

    template<typename BackendType>
    friend class ::__KageBackendRegistration;

    std::string m_name;

    public:

    /**
     * @brief Constructor.
     */
    Backend() = default;

    /**
     * @brief Move-constructor.
     */
    Backend(Backend&&) = default;

    /**
     * @brief Copy-constructor.
     */
    Backend(const Backend&) = default;

    /**
     * @brief Move-assignment operator.
     */
    Backend& operator=(Backend&&) = default;

    /**
     * @brief Copy-assignment operator.
     */
    Backend& operator=(const Backend&) = default;

    /**
     * @brief Destructor.
     */
    virtual ~Backend() = default;

    /**
     * @brief Return the name of backend.
     */
    const std::string& name() const {
        return m_name;
    }

    /**
     * @brief Returns a JSON-formatted configuration string.
     */
    virtual std::string getConfig() const = 0;

    /**
     * @brief Forward the input data to the backend and
     * call output_cb on the obtained output data.
     *
     * @param rpc_id ID of the RPC to forward out.
     * @param data Data to forward.
     * @param data_size Size of the data.
     * @param output_cb Callback to invoke on the output.
     *
     * @return a Result containing the result of the operation.
     */
    virtual Result<bool> forwardOutput(hg_id_t rpc_id, const char* data, size_t data_size,
                                       const std::function<void(const char*, size_t)>& output_cb) = 0;

    /**
     * @brief Set the InputProxy to which to redirect input RPCs.
     */
    virtual void setInputProxy(InputProxy proxy) = 0;

    /**
     * @brief Destroys the underlying proxy.
     *
     * @return a Result<bool> instance indicating
     * whether the database was successfully destroyed.
     */
    virtual Result<bool> destroy() = 0;

};

/**
 * @brief The ProxyFactory contains functions to create proxys.
 */
class ProxyFactory {

    template<typename BackendType>
    friend class ::__KageBackendRegistration;

    using json = nlohmann::json;

    public:

    ProxyFactory() = delete;

    /**
     * @brief Creates a proxy and returns a unique_ptr to the created instance.
     *
     * @param backend_name Name of the backend to use.
     * @param engine Thallium engine.
     * @param config Configuration object to pass to the backend's create function.
     * @param pool Optional pool in which to submit work.
     *
     * @return a unique_ptr to the created Proxy.
     */
    static std::unique_ptr<Backend> createProxy(const std::string& backend_name,
                                                const thallium::engine& engine,
                                                const json& config,
                                                const thallium::pool& pool);

    private:

    static std::unordered_map<std::string,
                std::function<std::unique_ptr<Backend>(
                    const thallium::engine&,
                    const json&,
                    const thallium::pool&)>> create_fn;
};

} // namespace kage


#define KAGE_REGISTER_BACKEND(__backend_name, __backend_type) \
    static __KageBackendRegistration<__backend_type> __kage ## __backend_name ## _backend( #__backend_name )

template<typename BackendType>
class __KageBackendRegistration {

    using json = nlohmann::json;

    public:

    __KageBackendRegistration(const std::string& backend_name)
    {
        kage::ProxyFactory::create_fn[backend_name] =
            [backend_name](const thallium::engine& engine,
                           const json& config,
                           const thallium::pool& pool) {
                auto p = BackendType::create(engine, config, pool);
                p->m_name = backend_name;
                return p;
            };
    }
};

#endif
