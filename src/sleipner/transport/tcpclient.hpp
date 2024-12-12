/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
/**
 * @file tcpclient.hpp
 * @brief Implements a socket client for TCP transport
 * @author Ferdinand Tonby-Strandborg
 */
#ifndef _SLEIPNER_TRANSPORT_TCPCLIENT_HPP_
#define _SLEIPNER_TRANSPORT_TCPCLIENT_HPP_

#include <memory>
#include <mutex>
#include <string>
#include <cstdint>

#include "sleipner/transport/isocket.hpp"
#include "sleipner/net/ip.hpp"

namespace sleipner::transport {
/**
 * @brief Client implementation for TCP network communication
 *
 * TcpClient provides an easy-to-use API for communicating via TCP.
 *
 * Basic usage example:
 * @code
 * IpAddress address = resolve("www.example.com", 8080);
 *
 * TcpClient client;
 * client.connect(address);
 *
 * client.send("Test...");
 *
 * // Wait up to 5 seconds for a reply
 * std::cout << "Received response: " << client.receive(1000, 5000) << std::endl;
 * @endcode
 *
 * @remarks It is made using the PImpl idiom such that the header does not need to include the
 *          system-specific libraries, such as @b WinSock
 */
class TcpClient: public ISocket {
protected:
    struct Impl;
    struct ImplCleanup { void operator()(Impl* ptr) const; };
    // struct ImplCleanup { void operator()(Impl* ptr) const; };

    std::unique_ptr<Impl, ImplCleanup> pimpl;
    mutable std::mutex    mutex;

public:
    /**
     * @brief Default constructor does not allow any operations to be carried out, except @b connect
     *
     * In order to send/receive data, a connection must first be created.
     * Until @b connect is called, all other operations result in @b SetupError.
     */
    TcpClient() = default;

    /**
     * @brief Default destructor cleans up all resources used
     */
    ~TcpClient();

    /**
     * @brief Establish TCP connection to host at the desired address
     *
     * @param address The address of the host
     * @throw std::invalid_argument If the address is obviously malformed
     * @throws ConnectionFailure
     * @throws SystemApiError
     */
    void connect(const net::IpAddress& address);

    /**
     * @brief Establish TCP connection to first connectable host at the desired address
     *
     * @param addresses The addresses to try to connect to
     * @throws std::invalid_argument If any address tried is obviously malformed
     * @throws ConnectionFailure
     * @throws SystemApiError
     */
    void connect(const std::vector<net::IpAddress>& addresses);

    /**
     * @brief Closes the TCP connection
     *
     * After calling @b close, the client will behave as if @b connect was never
     * called, and @b SetupError will be thrown as in an un-connected instance
     */
    void close() noexcept;

    /// @copydoc ISocket::connected()
    bool connected() const override;

    /// @copydoc ISocket::bytes_available()
    size_t bytes_available() const override;

    /// @copydoc ISocket::send(const char*, size_t)
    size_t send(const char* buf, size_t size) override;

    /// @copydoc ISocket::send(const std::string&)
    size_t send(const std::string& packet) override;

    /// @copydoc ISocket::receive(char*, size_t, uint64_t)
    size_t receive(char* buf, size_t size, uint64_t timeout) override;

    /// @copydoc ISocket::receive(size_t, uint64_t)
    std::string receive(size_t size, uint64_t timeout) override;

    /// @copydoc ISocket::peek(char*, size_t, uint64_t)
    size_t peek(char* buf, size_t size, uint64_t timeout) override;

    /// @copydoc ISocket::peek(size_t, uint64_t)
    std::string peek(size_t size, uint64_t timeout) override;
};
}

#endif
