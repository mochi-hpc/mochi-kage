/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "kage/ProxyHandle.hpp"
#include "kage/Result.hpp"
#include "kage/Exception.hpp"

#include "AsyncRequestImpl.hpp"
#include "ClientImpl.hpp"
#include "ProxyHandleImpl.hpp"

#include <thallium/serialization/stl/string.hpp>
#include <thallium/serialization/stl/pair.hpp>

namespace kage {

ProxyHandle::ProxyHandle() = default;

ProxyHandle::ProxyHandle(const std::shared_ptr<ProxyHandleImpl>& impl)
: self(impl) {}

ProxyHandle::ProxyHandle(const ProxyHandle&) = default;

ProxyHandle::ProxyHandle(ProxyHandle&&) = default;

ProxyHandle& ProxyHandle::operator=(const ProxyHandle&) = default;

ProxyHandle& ProxyHandle::operator=(ProxyHandle&&) = default;

ProxyHandle::~ProxyHandle() = default;

ProxyHandle::operator bool() const {
    return static_cast<bool>(self);
}

Client ProxyHandle::client() const {
    return Client(self->m_client);
}

void ProxyHandle::computeSum(
        int32_t x, int32_t y,
        int32_t* sum,
        AsyncRequest* req) const
{
    if(not self) throw Exception("Invalid kage::ProxyHandle object");
    auto& rpc = self->m_client->m_compute_sum;
    auto& ph  = self->m_ph;
    if(req == nullptr) { // synchronous call
        Result<int32_t> response = rpc.on(ph)(x, y);
        response.andThen([sum](int32_t s) { if(sum) *sum = s; });
    } else { // asynchronous call
        auto async_response = rpc.on(ph).async(x, y);
        auto async_request_impl =
            std::make_shared<AsyncRequestImpl>(std::move(async_response));
        async_request_impl->m_wait_callback =
            [sum](AsyncRequestImpl& async_request_impl) {
                Result<int32_t> response =
                    async_request_impl.m_async_response.wait();
                response.andThen([sum](int32_t s) { if(sum) *sum = s; });
            };
        *req = AsyncRequest(std::move(async_request_impl));
    }
}

}
