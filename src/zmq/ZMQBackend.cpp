/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "ZMQBackend.hpp"
#include <nlohmann/json-schema.hpp>
#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <iostream>

KAGE_REGISTER_BACKEND(zmq, ZMQProxy);

struct MessageContext {

    thallium::eventual<void>                        ev;
    const std::function<void(const char*, size_t)>& callback;
    kage::Result<bool>                              result;

    MessageContext(const std::function<void(const char*, size_t)>& cb)
    : callback{cb} {}
};

struct __attribute__ ((packed)) MessageHeader {
    MessageContext* sender_ctx;
    hg_id_t         rpc_id;
    bool            is_forward;
};

using nlohmann::json;
using nlohmann::json_schema::json_validator;

ZMQProxy::ZMQProxy(json&& config, thallium::pool pool,
                   zmq::context_t&& ctx,
                   zmq::socket_t&& pub_socket,
                   zmq::socket_t&& sub_socket)
: m_config(std::move(config))
, m_pool(std::move(pool))
, m_zmq_context(std::move(ctx))
, m_pub_socket(std::move(pub_socket))
, m_sub_socket(std::move(sub_socket))
{
    m_polling_ult = m_pool.make_thread([this]{ runPollingLoop(); });
}

std::string ZMQProxy::getConfig() const {
    return m_config.dump();
}

kage::Result<bool> ZMQProxy::forwardOutput(hg_id_t rpc_id, const char* input, size_t input_size,
                                           const std::function<void(const char*, size_t)>& output_cb) {
    auto context = MessageContext{output_cb};
    auto header = MessageHeader{&context, rpc_id, true};

    zmq::message_t header_msg{&header, sizeof(header)};
    zmq::message_t input_msg{input, input_size};

    m_pub_socket.send(header_msg, zmq::send_flags::sndmore);
    m_pub_socket.send(input_msg, zmq::send_flags::none);

    context.ev.wait();

    return context.result;
}

void ZMQProxy::setInputProxy(kage::InputProxy proxy) {
    m_input_proxy = std::move(proxy);
}

kage::Result<bool> ZMQProxy::destroy() {
    kage::Result<bool> result;
    m_need_stop.store(true);
    m_polling_ult->join();
    m_polling_ult.release();
    result.value() = true;
    return result;
}

std::unique_ptr<kage::Backend> ZMQProxy::create(
        const thallium::engine& engine,
        const json& config,
        const thallium::pool& pool) {
    (void)engine;
    static const json schema = R"(
    {
        "type": "object",
        "properties": {
            "pub_address": {"type": "string"},
            "sub_address": {"type": "string"}
        },
        "required": ["pub_address", "sub_address"]
    }
    )"_json;
    json_validator validator;
    validator.set_root_schema(schema);
    try {
        validator.validate(config);
    } catch(const std::exception& ex) {
        throw kage::Exception{
                fmt::format("While validating JSON config for ZMQ backend: {}", ex.what())};
    }

    auto& pub_address = config["pub_address"].get_ref<const std::string&>();
    auto& sub_address = config["sub_address"].get_ref<const std::string&>();

    auto final_config = json::object();
    final_config["pub_address"] = pub_address;
    final_config["sub_address"] = sub_address;

    bool pub_bind = pub_address.find('*') != std::string::npos;
    bool sub_bind = sub_address.find('*') != std::string::npos;

    try {
        zmq::context_t context{1};
        zmq::socket_t pub_socket{context, zmq::socket_type::pub};
        zmq::socket_t sub_socket{context, zmq::socket_type::sub};
        if(pub_bind)
            pub_socket.bind(pub_address);
        else
            pub_socket.connect(pub_address);
        if(sub_bind)
            sub_socket.bind(sub_address);
        else
            sub_socket.connect(sub_address);
        sub_socket.set(zmq::sockopt::subscribe, "");
        return std::unique_ptr<kage::Backend>(
            new ZMQProxy{
                std::move(final_config),
                pool,
                std::move(context),
                std::move(pub_socket),
                std::move(sub_socket)});
    } catch(const std::exception& ex) {
        throw kage::Exception{fmt::format("While initializing ZMQ: {}", ex.what())};
    }
}

void ZMQProxy::runPollingLoop() {
    while(!m_need_stop) {
        zmq::pollitem_t items[] = {{static_cast<void*>(m_sub_socket), 0, ZMQ_POLLIN, 0}};
        // Yield to give other ULTs an opportunity to run
        thallium::thread::yield();
        // Poll the socket with a timeout
        int rc = zmq::poll(items, 1, std::chrono::milliseconds(100));

        if (rc == -1) {
            spdlog::error("ZMQ's zmq::poll failed with error code {}", rc);
            continue;
        }

        if (rc == 0) {
            continue;
        }

        // Check if the socket has an incoming message
        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t msg;

            // Receive message from the other endpoint
            m_sub_socket.recv(msg, zmq::recv_flags::none);
            MessageHeader header;
            memcpy(&header, msg.data(), sizeof(header));

            m_sub_socket.recv(msg, zmq::recv_flags::none);
            const char* data      = static_cast<const char*>(msg.data());
            size_t      data_size = msg.size();

            if(header.is_forward) {
                // Received a "forward" request from other endpoint
                auto output_cb = [this, &header](const char* output, size_t output_size) {
                    // We are supposed to "echo" the header with "is_forward" set to false,
                    // alontg with our output data.
                    header.is_forward = false;

                    zmq::message_t header_msg{&header, sizeof(header)};
                    zmq::message_t output_msg{output, output_size};
                    m_pub_socket.send(header_msg, zmq::send_flags::sndmore);
                    m_pub_socket.send(output_msg, zmq::send_flags::none);

                    // FIXME: the above call is blocking, maybe find a way to make it not block
                };
                m_input_proxy.forwardInput(header.rpc_id, data, data_size, output_cb);
            } else {
                // Received the response for an RPC we have forwarded
                MessageContext* sender_ctx = header.sender_ctx;
                sender_ctx->callback(data, data_size);
                sender_ctx->ev.set_value();
            }
        }
    }
}
