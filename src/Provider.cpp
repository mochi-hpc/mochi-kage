/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "kage/Provider.hpp"

#include "ProviderImpl.hpp"

#include <thallium/serialization/stl/string.hpp>

namespace tl = thallium;

namespace kage {

Provider::Provider(const tl::engine& engine,
                   uint16_t provider_id,
                   const std::string& config,
                   const tl::provider_handle& target,
                   const tl::pool& rpc_pool,
                   const tl::pool& proxy_pool)
: self(std::make_shared<ProviderImpl>(
        engine, provider_id, config, target, rpc_pool, proxy_pool)) {
    self->get_engine().push_finalize_callback(this, [p=this]() { p->self.reset(); });
}

Provider::Provider(Provider&& other) {
    other.self->get_engine().pop_finalize_callback(this);
    self = std::move(other.self);
    self->get_engine().push_finalize_callback(this, [p=this]() { p->self.reset(); });
}

Provider::~Provider() {
    if(self) {
        self->get_engine().pop_finalize_callback(this);
    }
}

std::string Provider::getConfig() const {
    return self ? self->getConfig() : "{}";
}

Provider::operator bool() const {
    return static_cast<bool>(self);
}

}
