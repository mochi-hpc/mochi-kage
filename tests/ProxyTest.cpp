/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>
#include "Ensure.hpp"
#include <kage/Provider.hpp>

TEST_CASE("Proxy test", "[proxy]") {
    auto engine = thallium::engine("na+sm", THALLIUM_SERVER_MODE);
    ENSURE(engine.finalize());
    const auto provider_config = R"(
    {
        "exported_rpcs": [],
        "direction": "out",
        "proxy": {
            "type": "echo",
            "config": {}
        }
    }
    )";
    kage::Provider provider(engine, 42, provider_config);
}
