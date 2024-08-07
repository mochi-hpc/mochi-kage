/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>
#include "Ensure.hpp"
#include <kage/Client.hpp>
#include <kage/Provider.hpp>
#include <kage/ProxyHandle.hpp>

TEST_CASE("Client test", "[client]") {

    auto engine = thallium::engine("na+sm", THALLIUM_SERVER_MODE);
    ENSURE(engine.finalize());
    // Initialize the provider
    const auto provider_config = R"(
    {
        "proxy": {
            "type": "dummy",
            "config": {}
        }
    }
    )";

    kage::Provider provider(engine, 42, provider_config);

    SECTION("Open proxy") {

        kage::Client client(engine);
        std::string addr = engine.self();

        kage::ProxyHandle my_proxy = client.makeProxyHandle(addr, 42);
        REQUIRE(static_cast<bool>(my_proxy));

        REQUIRE_THROWS_AS(client.makeProxyHandle(addr, 55), kage::Exception);
        REQUIRE_NOTHROW(client.makeProxyHandle(addr, 55, false));
    }
}
