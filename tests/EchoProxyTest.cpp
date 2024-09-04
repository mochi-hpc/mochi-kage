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

TEST_CASE("EchoProxy test", "[echo]") {
    auto engine = thallium::engine("na+sm", THALLIUM_SERVER_MODE);
    ENSURE(engine.finalize());
    const auto provider_config = R"(
    {
        "exported_rpcs": ["my_rpc"],
        "direction": "out",
        "proxy": {
            "type": "echo",
            "config": {}
        }
    }
    )";
    kage::Provider provider(engine, 42, "kage", provider_config);

    auto rpc = engine.define("my_rpc");

    std::string input = "Matthieu Dorier";
    auto ph = thallium::provider_handle{engine.self(), 42};
    std::string output = rpc.on(ph)(input);
    REQUIRE(input == output);
}
