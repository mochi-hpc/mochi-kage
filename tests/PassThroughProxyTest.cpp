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
        std::string result = "Hello " + name;
        req.respond(result);
    }
};

TEST_CASE("PassThroughProxy test", "[passthrough]") {
    auto engine = thallium::engine("na+sm", THALLIUM_SERVER_MODE);
    ENSURE(engine.finalize());
    const auto provider_config = R"(
    {
        "exported_rpcs": ["hello"],
        "direction": "inout",
        "proxy": {
            "type": "passthrough",
            "config": {}
        }
    }
    )";

    auto input_provider = new my_input_provider{engine, 33};
    engine.push_finalize_callback([input_provider]() { delete input_provider; });

    kage::Provider provider{
        engine, 42, "kage", provider_config,
        thallium::provider_handle{engine.self(), 33}
    };

    auto hello = engine.define("hello");

    std::string input = "Matthieu Dorier";
    auto ph = thallium::provider_handle{engine.self(), 42};
    std::string output = hello.on(ph)(input);
    REQUIRE(output == "Hello Matthieu Dorier");
}
