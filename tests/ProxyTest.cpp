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

TEST_CASE("Proxy test", "[proxy]") {
    auto engine = thallium::engine("na+sm", THALLIUM_SERVER_MODE);
    ENSURE(engine.finalize());
    const auto provider_config = R"(
    {
        "proxy": {
            "type": "dummy",
            "config": {}
        }
    }
    )";
    kage::Provider provider(engine, 42, provider_config);

    SECTION("Create ProxyHandle") {
        kage::Client client(engine);
        std::string addr = engine.self();

        auto rh = client.makeProxyHandle(addr, 42);

        SECTION("Send Sum RPC") {
            int32_t result = 0;
            REQUIRE_NOTHROW(rh.computeSum(42, 51, &result));
            REQUIRE(result == 93);

            REQUIRE_NOTHROW(rh.computeSum(42, 51));

            kage::AsyncRequest request;
            REQUIRE_NOTHROW(rh.computeSum(42, 52, &result, &request));
            REQUIRE_NOTHROW(request.wait());
            REQUIRE(result == 94);
        }
    }
}
