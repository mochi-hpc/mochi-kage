/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "MargoBackend.hpp"
#include <nlohmann/json-schema.hpp>
#include <spdlog/spdlog.h>
#include <iostream>
#include <thallium/serialization/stl/string.hpp>

KAGE_REGISTER_BACKEND(margo, MargoProxy);

using nlohmann::json;
using nlohmann::json_schema::json_validator;

MargoProxy::MargoProxy(json&& config, thallium::engine internal_engine, thallium::endpoint remote_endpoint)
: m_config(std::move(config))
, m_internal_engine(std::move(internal_engine))
, m_remote_endpoint(std::move(remote_endpoint))
{
    // FIXME: this can be improved with callback-driven piping of input and output
    // instead of converting to intermediate std::strings.
    if(m_internal_engine.is_listening()) {
        std::function<void(const thallium::request&, hg_id_t, const std::string&)> rpc =
            [this](const thallium::request& req, hg_id_t rpc_id, const std::string& input) {
                auto output_cb = [&](const char* output, size_t output_size) {
                    req.respond(std::string{output, output_size});
                };
                m_input_proxy.forwardInput(rpc_id, input.data(), input.size(), output_cb);
            };
        m_rpc = m_internal_engine.define("kage_forward", rpc);
    } else {
        m_rpc = m_internal_engine.define("kage_forward");
    }
}

std::string MargoProxy::getConfig() const {
    return m_config.dump();
}

kage::Result<bool> MargoProxy::forwardOutput(hg_id_t rpc_id, const char* input, size_t input_size,
                                             const std::function<void(const char*, size_t)>& output_cb) {
    // FIXME: this can be improved with callback-driven piping of input and output
    // instead of converting to intermediate std::strings.
    std::string output = m_rpc.on(m_remote_endpoint)(rpc_id, std::string{input, input_size});
    output_cb(output.data(), output.size());
    return kage::Result<bool>{};
}

void MargoProxy::setInputProxy(kage::InputProxy proxy) {
    m_input_proxy = std::move(proxy);
}

kage::Result<bool> MargoProxy::destroy() {
    m_rpc.deregister();
    m_remote_endpoint = thallium::endpoint{};
    m_internal_engine.finalize();
    m_internal_engine = thallium::engine{};
    kage::Result<bool> result;
    result.value() = true;
    return result;
}

std::unique_ptr<kage::Backend> MargoProxy::create(
        const thallium::engine& engine,
        const json& config,
        const thallium::pool& pool) {
    (void)engine;
    static const json schema = R"(
    {
        "type": "object",
        "properties": {
            "listening": {"type": "boolean"},
            "address": {"type": "string"},
            "remote_address": {"type": "string"}
        },
        "required": ["listening", "address", "remote_address"]
    }
    )"_json;
    json_validator validator;
    validator.set_root_schema(schema);
    try {
        validator.validate(config);
    } catch(const std::exception& ex) {
        throw kage::Exception{
                fmt::format("While validating JSON config for Margo backend: {}", ex.what())};
    }

    auto& address = config["address"].get_ref<const std::string&>();
    auto& remote_address = config["remote_address"].get_ref<const std::string&>();
    auto listening = config["listening"].get<bool>();

    try {
        margo_init_info info = {
            /* .json_config = */ nullptr,
            /* .progress_pool = */ pool.native_handle(),
            /* .rpc_pool = */ pool.native_handle(),
            /* .hg_class = */ nullptr,
            /* .hg_context = */ nullptr,
            /* .hg_init_info = */ nullptr,
            /* .logger = */ nullptr,
            /* .monitor = */ nullptr
        };
        auto internal_engine = thallium::engine{
            address,
            listening ? THALLIUM_SERVER_MODE : THALLIUM_CLIENT_MODE,
            &info};
        auto remote_endpoint = internal_engine.lookup(remote_address);

        auto final_config = json::object();
        final_config["address"] = static_cast<std::string>(internal_engine.self());
        final_config["remote_address"] = static_cast<std::string>(remote_endpoint);
        final_config["listening"] = listening;

        return std::unique_ptr<kage::Backend>(
            new MargoProxy{
                std::move(final_config),
                std::move(internal_engine),
                std::move(remote_endpoint)});
    } catch(const std::exception& ex) {
        throw kage::Exception{fmt::format("While initializing Margo: {}", ex.what())};
    }
}
