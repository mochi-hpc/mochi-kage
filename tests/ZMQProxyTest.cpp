/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>
#include "Ensure.hpp"
#include <thallium/serialization/stl/string.hpp>
#include <kage/Provider.hpp>
#include <spdlog/spdlog.h>

class my_input_provider : public thallium::provider<my_input_provider> {

    thallium::auto_remote_procedure m_hello;

    public:

    my_input_provider(
        thallium::engine engine,
        uint16_t provider_id)
    : thallium::provider<my_input_provider>{engine, provider_id}
    , m_hello{define("hello", &my_input_provider::hello)}
    {}

    void hello(const thallium::request& req, const std::string& name) {
        auto provider_id = get_provider_id();
        std::string result = "Hello " + name + " from provider " + std::to_string(provider_id);
        req.respond(result);
    }
};

TEST_CASE("ZMQProxy test", "[zmq]") {
    auto engine = thallium::engine("na+sm", THALLIUM_SERVER_MODE);
    ENSURE(engine.finalize());

    spdlog::set_level(spdlog::level::from_str("trace"));

    const auto provider_config_1 = R"(
    {
        "exported_rpcs": ["hello"],
        "direction": "inout",
        "proxy": {
            "type": "zmq",
            "config": {
                "pub_address": "tcp://*:4555",
                "sub_address": "tcp://*:4556"
            }
        }
    }
    )";

    const auto provider_config_2 = R"(
    {
        "exported_rpcs": ["hello"],
        "direction": "inout",
        "proxy": {
            "type": "zmq",
            "config": {
                "pub_address": "tcp://localhost:4556",
                "sub_address": "tcp://localhost:4555"
            }
        }
    }
    )";

    auto input_provider_1 = new my_input_provider{engine, 33};
    engine.push_finalize_callback([input_provider_1]() { delete input_provider_1; });

    auto input_provider_2 = new my_input_provider{engine, 34};
    engine.push_finalize_callback([input_provider_2]() { delete input_provider_2; });

    kage::Provider provider1{
        engine, 42, provider_config_1,
        thallium::provider_handle{engine.self(), 33}
    };

    kage::Provider provider2{
        engine, 43, provider_config_2,
        thallium::provider_handle{engine.self(), 34}
    };

    // sleep a bit to allow the providers to connect to each other
    // before we start sending things
    thallium::thread::sleep(engine, 200);

    // with the setup above, RPCs sent to Kage provider 42 will end up
    // forwarded to my_input_provider 34, and RPCs sent to Kage provider
    // 43 will end up forwarded to my_input_provider 33.
    auto hello = engine.define("hello");
    {
        std::string input = "Matthieu Dorier";
        auto ph = thallium::provider_handle{engine.self(), 42};
        std::string output = hello.on(ph)(input);
        REQUIRE(output == "Hello Matthieu Dorier from provider 34");
    }
    std::cerr << "----------" << std::endl;
    {
        std::string input = "Matthieu Dorier";
        auto ph = thallium::provider_handle{engine.self(), 43};
        std::string output = hello.on(ph)(input);
        REQUIRE(output == "Hello Matthieu Dorier from provider 33");
    }
}
