/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
/**
 * @file isocket.hpp
 * @brief Interface for socket clients
 * @author Ferdinand Tonby-Strandborg
 */
#ifndef _SLEIPNER_ISOCKET_HPP_
#define _SLEIPNER_ISOCKET_HPP_

#include <string>
#include <cstdint>

namespace sleipner::transport {
/**
 * @brief Interface for a socket client.
 *
 * ISocket provides an abstract implementation for the methods that all "sockets"
 * provided by this library, in order to send, receive and peek at data.
 *
 * @note The derived classes do not necessarily provide sockets, but rather socket-like
 *       control of various data transfer targets, such as COM Ports.
 *
 * @note The derived classes do not strictly follow @b RAII. They are guaranteed to clan
 *       all resources on destruction, but might not acquire resources on construction.
 *       As such, @b SetupError can be thrown by any method in this interface, indicating
 *       that a setup step was missed, or incorrectly carried out - for example a @b TcpClient
 *       was never connected before trying to send data.
 *
 * @note The derived classes are intended as a socket set-up using @b connect or @b accept.
 *       The rationale for this is that these are the socket instances that send/receive data,
 *       which is the purpose of this interface. This means that no real @a Server classes
 *       that use @b bind behaviour are derived.
 */
class ISocket {
protected:
    /// @brief Protected constructor to prevent instance being created
    ISocket() = default;

public:
    /// @brief Require a proper destructor to clean up resources
    virtual ~ISocket() = default;
    /**
     * @brief Check if the socket is still connected
     *
     * @throws SetupError
     * @throws SystemApiError
     */
    virtual bool connected() const = 0;

    /**
     * @brief Retrieve count of bytes available to read
     *
     * @throws SocketDisconnection
     * @throws SetupError
     * @throws SystemApiError
     */
    virtual size_t bytes_available() const = 0;

    /**
     * @brief Sends data over the connection
     *
     * @param [in] buf Byte buffer containing data to send
     * @param [in] size Size of the byte buffer/number of bytes to send
     * @throws SocketDisconnection
     * @throws SetupError
     * @throws SystemApiError
     * @throws std::overflow_error
     * @return size_t Number of bytes sent
     */
    virtual size_t send(const char* buf, size_t size) = 0;

    /**
     * @brief Sends data over the connection
     *
     * @param [in] packet Data to send
     * @throws SocketDisconnection
     * @throws SetupError
     * @throws SystemApiError
     * @throws std::overflow_error
     * @return size_t Number of bytes sent
     */
    virtual size_t send(const std::string& packet) = 0;

    /**
     * @brief Receive data from the connection
     *
     * @param [inout] buf Byte buffer to store the data
     * @param [in] size Size of the buffer/max number of bytes to receive
     * @param [in] timeout Milliseconds to block if no data available
     * @throws SocketDisconnection
     * @throws SetupError
     * @throws SystemApiError
     * @return size_t Number of bytes received
     */
    virtual size_t receive(char* buf, size_t size, uint64_t timeout) = 0;

    /**
     * @brief Receive data from the connection
     *
     * @param [in] size Max number of bytes to receive
     * @param [in] timeout Milliseconds to block if no data available
     * @throws SocketDisconnection
     * @throws SetupError
     * @throws SystemApiError
     * @return std::string Bytes received
     */
    virtual std::string receive(size_t size, uint64_t timeout) = 0;

    /**
     * @brief Peek data from the connection - this is still available for the next call to @b receive
     *
     * @param [in] buf Byte buffer to store the data
     * @param [in] size Size of the buffer/max number of bytes to receive
     * @param [in] timeout Milliseconds to block if no data available
     * @throws SocketDisconnection
     * @throws SetupError
     * @throws SystemApiError
     * @return size_t Number of bytes peeked
     */
    virtual size_t peek(char* buf, size_t size, uint64_t timeout) = 0;

    /**
     * @brief Peek data from the connection - this is still available for the next call to @b receive
     *
     * @param [in] size Max number of bytes to receive
     * @param [in] timeout Milliseconds to block if no data available
     * @throws SocketDisconnection
     * @throws SetupError
     * @throws SystemApiError
     * @return std::string Bytes received
     */
    virtual std::string peek(size_t size, uint64_t timeout) = 0;
};
}

#endif
