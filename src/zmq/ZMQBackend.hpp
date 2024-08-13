/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __ZMQ_BACKEND_HPP
#define __ZME_BACKEND_HPP

#include <zmq.hpp>
#include <kage/Backend.hpp>

using json = nlohmann::json;

/**
 * ZMQ implementation of an kage Backend.
 */
class ZMQProxy : public kage::Backend {

    json             m_config;
    thallium::pool   m_pool;
    kage::InputProxy m_input_proxy;
    zmq::context_t   m_zmq_context;
    zmq::socket_t    m_pub_socket;
    zmq::socket_t    m_sub_socket;

    std::atomic<bool>                   m_need_stop{false};
    thallium::managed<thallium::thread> m_polling_ult;

    public:

    /**
     * @brief Constructor.
     */
    ZMQProxy(json&& config,
             thallium::pool pool,
             zmq::context_t&& ctx,
             zmq::socket_t&& pub_socket,
             zmq::socket_t&& sub_socket);

    /**
     * @brief Move-constructor.
     */
    ZMQProxy(ZMQProxy&&) = delete;

    /**
     * @brief Copy-constructor.
     */
    ZMQProxy(const ZMQProxy&) = delete;

    /**
     * @brief Move-assignment operator.
     */
    ZMQProxy& operator=(ZMQProxy&&) = delete;

    /**
     * @brief Copy-assignment operator.
     */
    ZMQProxy& operator=(const ZMQProxy&) = delete;

    /**
     * @brief Destructor.
     */
    virtual ~ZMQProxy() = default;

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
     * create a ZMQProxy.
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

    private:

    void runPollingLoop();
};

#endif
