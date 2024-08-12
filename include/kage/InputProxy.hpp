/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __KAGE_INPUT_PROXY_HPP
#define __KAGE_INPUT_PROXY_HPP

#include <kage/Result.hpp>
#include <thallium.hpp>
#include <memory>

namespace kage {

namespace tl = thallium;

class Provider;
class ProviderImpl;

/**
 * @brief An InputProxy is an object holding a reference to
 * a Provider and abstracting the forwarding of input RPC away
 * from the Backend implementations.
 */
class InputProxy {

    public:

    /**
     * @brief Default-constructor.
     */
    InputProxy() = default;

    /**
     * @brief Copy-constructor is deleted.
     */
    InputProxy(const InputProxy&) = default;

    /**
     * @brief Move-constructor.
     */
    InputProxy(InputProxy&&) = default;

    /**
     * @brief Copy-assignment operator is deleted.
     */
    InputProxy& operator=(const InputProxy&) = default;

    /**
     * @brief Move-assignment operator is deleted.
     */
    InputProxy& operator=(InputProxy&&) = default;

    /**
     * @brief Destructor.
     */
    ~InputProxy();

    /**
     * @brief Checks whether the InputProxy instance is valid.
     */
    operator bool() const;

    /**
     * @brief Forward the input data to the proxy,
     * and call output_cb on the obtained output data.
     *
     * @param rpc_id ID of the RPC to forward.
     * @param data Data to forward.
     * @param data_size Size of the data.
     * @param output_cb Callback to invoke on the output.
     *
     * @return a Result containing the result of the operation.
     */
    Result<bool> forwardInput(hg_id_t rpc_id, const char* data, size_t data_size,
                              const std::function<void(const char*, size_t)>& output_cb);

    private:

    friend class Provider;

    std::weak_ptr<ProviderImpl> self;

    InputProxy(std::shared_ptr<ProviderImpl> impl);
};

}

#endif
