/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __KAGE_PROXY_HANDLE_HPP
#define __KAGE_PROXY_HANDLE_HPP

#include <thallium.hpp>
#include <memory>
#include <unordered_set>
#include <nlohmann/json.hpp>
#include <kage/Client.hpp>
#include <kage/Exception.hpp>
#include <kage/AsyncRequest.hpp>

namespace kage {

namespace tl = thallium;

class Client;
class ProxyHandleImpl;

/**
 * @brief A ProxyHandle object is a handle for a remote proxy
 * on a server. It enables invoking the proxy's functionalities.
 */
class ProxyHandle {

    friend class Client;

    public:

    /**
     * @brief Constructor. The resulting ProxyHandle handle will be invalid.
     */
    ProxyHandle();

    /**
     * @brief Copy-constructor.
     */
    ProxyHandle(const ProxyHandle&);

    /**
     * @brief Move-constructor.
     */
    ProxyHandle(ProxyHandle&&);

    /**
     * @brief Copy-assignment operator.
     */
    ProxyHandle& operator=(const ProxyHandle&);

    /**
     * @brief Move-assignment operator.
     */
    ProxyHandle& operator=(ProxyHandle&&);

    /**
     * @brief Destructor.
     */
    ~ProxyHandle();

    /**
     * @brief Returns the client this proxy has been opened with.
     */
    Client client() const;


    /**
     * @brief Checks if the ProxyHandle instance is valid.
     */
    operator bool() const;

    /**
     * @brief Requests the target proxy to compute the sum of two numbers.
     * If result is null, it will be ignored. If req is not null, this call
     * will be non-blocking and the caller is responsible for waiting on
     * the request.
     *
     * @param[in] x first integer
     * @param[in] y second integer
     * @param[out] result result
     * @param[out] req request for a non-blocking operation
     */
    void computeSum(int32_t x, int32_t y,
                    int32_t* result = nullptr,
                    AsyncRequest* req = nullptr) const;

    private:

    /**
     * @brief Constructor is private. Use a Client object
     * to create a ProxyHandle instance.
     *
     * @param impl Pointer to implementation.
     */
    ProxyHandle(const std::shared_ptr<ProxyHandleImpl>& impl);

    std::shared_ptr<ProxyHandleImpl> self;
};

}

#endif
