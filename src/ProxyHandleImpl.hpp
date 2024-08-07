/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __KAGE_PROXY_HANDLE_IMPL_H
#define __KAGE_PROXY_HANDLE_IMPL_H

#include "ClientImpl.hpp"

namespace kage {

class ProxyHandleImpl {

    public:

    std::shared_ptr<ClientImpl> m_client;
    tl::provider_handle         m_ph;

    ProxyHandleImpl() = default;

    ProxyHandleImpl(std::shared_ptr<ClientImpl> client,
                       tl::provider_handle&& ph)
    : m_client(std::move(client))
    , m_ph(std::move(ph)) {}
};

}

#endif
