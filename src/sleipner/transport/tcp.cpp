/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
#ifndef _WIN32
#error "tcpclient.cpp Not yet implemented!"
#endif

#include "sleipner/transport/tcpclient.hpp"
#include "sleipner/transport/error.hpp"
#include "sleipner/sys/error.hpp"

#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #include "sleipner/sys/winsock.hpp"

    #ifndef VALIDATE_SOCKET
    #define VALIDATE_SOCKET(socket) (socket != INVALID_SOCKET)
    #endif

    #ifndef SOCKET_FAILURE
    #define SOCKET_FAILURE(res) (res == SOCKET_ERROR)
    #endif

#else
    #error "Still not implemented..."

    #ifndef INVALID_SOCKET
    #define INVALID_SOCKET -1
    #endif

    #ifndef VALIDATE_SOCKET
    #define VALIDATE_SOCKET(socket) (socket >= 0)
    #endif

    #ifndef SOCKET_FAILURE
    #define SOCKET_FAILURE(res) (res < 0)
    #endif
#endif

namespace sleipner::transport {
#ifdef _WIN32
    typedef SOCKET socket_t;
#else
    typedef int socket_t;
#endif

/********************************************/
/* System specific socket methods           */
/********************************************/
static void _new_socket(socket_t& socket, int family) {
    if ( VALIDATE_SOCKET(socket) )
        throw error::SetupError("TCP socket already setup!");

    if ( family != AF_INET && family != AF_INET6 )
        throw std::invalid_argument("Invalid address family!");

    socket = ::socket(family, SOCK_STREAM, 0);

    // Should not fail - all socket inputs are guaranteed to be valid...
    if ( !VALIDATE_SOCKET(socket) )
        throw error::SystemApiError(::WSAGetLastError());
}

static void _close_socket(socket_t& socket) noexcept {
    // Ignore errors in closing...
    if ( VALIDATE_SOCKET(socket) )
        #ifdef _WIN32
            ::closesocket(socket);
        #else
            ::close(socket);
        #endif
    socket = INVALID_SOCKET;
}

static void _connect(socket_t& socket, const net::IpAddress& address) {
    if ( address.addr.size() != sizeof(::sockaddr_in) && address.addr.size() != sizeof(::sockaddr_in6) )
        throw std::invalid_argument("Invalid address structure!");

    ::sockaddr* addr = reinterpret_cast<::sockaddr*>(const_cast<char*>(address.addr.data()));

    int res = ::connect(socket, addr, address.addr.size());

    if ( SOCKET_FAILURE(res) ) {
        int err = ::WSAGetLastError();
        _close_socket(socket);

        switch ( err ) {
            case WSAEWOULDBLOCK:   // Pending completion
                /// @todo...
                throw std::runtime_error("Incomplete handling not yet implemented!");

            case WSAENETDOWN:      // Dead network
            case WSAEADDRINUSE:    // Address "occupied"
            case WSAEINTR:         // Interrupted by blocking call -> try again
            case WSAEINPROGRESS:   // Blocking operation currently executing
            case WSAEALREADY:      // Socket already has active operation
            case WSAEADDRNOTAVAIL: // Address not valid
            case WSAECONNREFUSED:  // Connection rejected
            case WSAENETUNREACH:   // Unreachable network
            case WSAEHOSTUNREACH:  // Unreachable host
            case WSAETIMEDOUT:     // No response within reasonable time
                throw error::ConnectionFailure(sys::error_message(err));

            case WSAEAFNOSUPPORT:  // Address incompatible with protocol - should not happen...
            case WSAEFAULT:        // Invalid address pointer
            case WSAEINVAL:        // Invalid argument
                throw std::invalid_argument(sys::error_message(err));

            // case WSANOTINITIALIZED:
            // case WSAEISCONN:       // Socket already connected -> Should not happen
            // case WSAENOBUFS:       // Buffer/queue full
            // case WSAENOTSOCK:      // Operation on something that is not a socket
            // case WSAEACCES:        // Access forbidden
            default:
                throw error::SystemApiError(err);
        }
    }
}

static size_t _send(socket_t& socket, const char* data, size_t size) {
    if ( !VALIDATE_SOCKET(socket) )
        throw error::SetupError("TCP socket not connected!");

    int res = ::send(socket, data, size, 0);

    if ( SOCKET_FAILURE(res) ) {
        int err = ::WSAGetLastError();

        switch ( err ) {
            case WSAENETDOWN:
            case WSAENETRESET:
            case WSAENOTCONN:
            case WSAEHOSTUNREACH:
            case WSAECONNABORTED:
            case WSAECONNRESET:
            case WSAETIMEDOUT:
                throw error::SocketDisconnection(sys::error_message(err));

            case WSAEMSGSIZE:
                throw std::overflow_error(sys::error_message(err));

            case WSAEWOULDBLOCK:
                throw std::runtime_error("Incomplete handling/retries not yet implemented!");

            // case WSAEINVAL:
            // case WSAESHUTDOWN:
            // case WSAEOPNOTSUPP:
            // case WSAENOTSOCK:
            // case WSAENOBUFS:
            // case WSAEFAULT:
            // case WSAEINTR: // Interrupted through WSACancelBlockingCall
            // case WSANOTINITIALISED:
            // case WSAEACCES:
            default:
                throw error::SystemApiError(err);
        }
    }

    return (unsigned int) res;
}

static unsigned int _select(const socket_t& socket, uint64_t timeout) {
    #ifdef _WIN32
        int nfd = 0;
    #else
        int nfd = socket + 1;
    #endif

    ::fd_set readfds;
    ::timeval tv {0};

    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);

    int res = ::select(nfd, &readfds, nullptr, nullptr, &tv);

    /// @todo - Map "valid" errors
    if ( SOCKET_FAILURE(res) ) {
        int err = ::WSAGetLastError();

        switch ( err ) {
            case WSAENETDOWN:
                throw error::ConnectionFailure(sys::error_message(err));

            case WSAEINPROGRESS:
                throw std::runtime_error("Incomplete handling/retry not yet implemented!");

            // case WSAENOTSOCK:
            // case WSAEINTR: // Interrupted through WSACancelBlockingCall
            // case WSAEFAULT:
            // case WSANOTINITIALISED:
            default:
                throw error::SystemApiError(err);
        }
    }

    return res;
}

static bool _connected(const socket_t& socket) {
    #ifdef _WIN32
        int nfd = 0;
    #else
        int nfd = socket + 1;
    #endif

    unsigned int s;
    try {
        s = _select(socket, 0);
    } catch ( error::ConnectionFailure ) {
        return false;
    }

    if ( s > 0 ) {
        char buf;
        int res = ::recv(socket, &buf, 1, MSG_PEEK);

        if ( SOCKET_FAILURE(res) ) {
            int err = ::WSAGetLastError();
            switch ( err ) {
                case WSAENETDOWN:
                case WSAENOTCONN:
                case WSAENETRESET:
                case WSAECONNABORTED:
                case WSAETIMEDOUT:
                case WSAECONNRESET:
                    return false;

                case WSAEWOULDBLOCK:
                case WSAEINPROGRESS:
                    throw std::runtime_error("Incomplete/retry handling not implemented!");

                default:
                    throw error::SystemApiError(err);
            }
        }

        return res > 0;
    }

    int res;
    #ifdef _WIN32
        int len = sizeof(res);
    #else
        ::socklen_t len = sizeof(res);
    #endif

    int err = ::getsockopt(socket, SOL_SOCKET, SO_ERROR, (char*)&res, &len);

    if ( SOCKET_FAILURE(err) ) {
        int err = ::WSAGetLastError();

        switch ( err ) {
            case WSAENETDOWN:
                return false;

            case WSAEINPROGRESS:
                throw std::runtime_error("Incomplete/retry handling not implemented!");

            // case WSAEFAULT:
            // case WSAEINVAL:
            // case WSAENOPROTOOPT:
            // case WSAENOTSOCK:
            // case WSANOTINITIALISED:
            default:
                throw error::SystemApiError(err);
        }
    }

    return res == 0;
}

static size_t _bytes_available(const socket_t& socket, uint64_t timeout) {
    #ifdef _WIN32
        int nfd = 0;
    #else
        int nfd = socket + 1;
    #endif

    if ( _select(socket, timeout) == 0 )
        return 0;

    unsigned long bytes_avail = 0;
    #ifdef _WIN32
        int res = ::ioctlsocket(socket, FIONREAD, &bytes_avail);
    #else
        int res = ::ioctl(socket, FIONREAD, &bytes_avail);
    #endif

    if ( SOCKET_FAILURE(res) ) {
        int err = ::WSAGetLastError();

        switch ( err ) {
            case WSAENETDOWN:
                throw error::SocketDisconnection(sys::error_message(err));

            case WSAEINPROGRESS:
                throw std::runtime_error("Incomplete/retry handling not implemented!");

            // case WSANOTINITIALISED:
            // case WSAENOTSOCK:
            // case WSAEFAULT:
            default:
                throw error::SystemApiError(err);
        }
    }

    return bytes_avail;
}

static size_t _recv(socket_t& socket, char* buf, size_t size, uint64_t timeout, bool peek) {
    if ( !VALIDATE_SOCKET(socket) )
        throw error::SetupError("TCP socket not connected!");

    if ( _select(socket, timeout) == 0 )
        return 0;

    int res = ::recv(socket, buf, size, peek ? MSG_PEEK : 0);

    if ( SOCKET_FAILURE(res) ) {
        int err = ::WSAGetLastError();

        switch ( err ) {
            case WSAENETDOWN:
            case WSAENOTCONN:
            case WSAENETRESET:
            case WSAECONNABORTED:
            case WSAETIMEDOUT:
            case WSAECONNRESET:
                throw error::SocketDisconnection(sys::error_message(err));

            case WSAEMSGSIZE: // Buffer not big enough for all data... - truncated data retrieved
                throw std::overflow_error(sys::error_message(err));

            case WSAEWOULDBLOCK:
            case WSAEINPROGRESS:
                throw std::runtime_error("Incomplete/retry handling not implemented!");

            // throw WSAEINVAL:
            // case WSAESHUTDOWN:
            // case WSAEOPTNOTSUPP:
            // case WSAENOTSOCK:
            // case WSAEINTR:
            // case WSAEFAULT:
            // case WSANOTINITIALISED:
            default:
                throw error::SystemApiError(err);
        }
    }

    if ( res == 0 && size != 0 )
        throw error::SocketDisconnection("Socket disconnected gracefully!");

    return (unsigned int) res;
}


/********************************************/
/* TcpClient::Impl                          */
/********************************************/
struct TcpClient::Impl {
    protected:
        socket_t socket = INVALID_SOCKET;

        #ifdef _WIN32
            sys::WinsockLoader wsl;
        #endif

    public:
        void connect(const net::IpAddress& address) {
            _new_socket(socket, address.family);
            _connect(socket, address);
        }

        void connect(const std::vector<net::IpAddress>& addresses) {
            for ( auto a: addresses ) {
                try {
                    _new_socket(socket, a.family);
                    _connect(socket, a);
                    return;
                } catch ( error::ConnectionFailure& e ) {
                    /* Try next address */
                }
            }
            throw error::ConnectionFailure("Could not connect to any given address!");
        }

        void close() noexcept {
            _close_socket(socket);
        }

        bool connected() const {
            return _connected(socket);
        }

        size_t bytes_available() const {
            return _bytes_available(socket, 0);
        }

        size_t send(const char* buf, size_t size) {
            return _send(socket, buf, size);
        }

        size_t receive(char* buf, size_t size, uint64_t timeout) {
            return _recv(socket, buf, size, timeout, false);
        }

        size_t peek(char* buf, size_t size, uint64_t timeout) {
            return _recv(socket, buf, size, timeout, true);
        }

    public:
        Impl() = default;
        ~Impl() {
            close();
        }
};


void TcpClient::ImplCleanup::operator()(Impl* ptr) const {
    if ( ptr )
        delete ptr;
}

/********************************************/
/* TcpClient                                */
/********************************************/
TcpClient::~TcpClient() = default;

void TcpClient::connect(const net::IpAddress& address) {
    std::lock_guard lock(mutex);
    if ( pimpl )
        throw error::SetupError("TcpClient already connected!");
    pimpl.reset(new Impl());
    pimpl->connect(address);
}

void TcpClient::connect(const std::vector<net::IpAddress>& addresses) {
    std::lock_guard lock(mutex);
    if ( pimpl )
        throw error::SetupError("TcpClient already connected!");
    pimpl.reset(new Impl());
    pimpl->connect(addresses);
}

void TcpClient::close() noexcept {
    std::lock_guard lock(mutex);
    pimpl.reset(nullptr);
}

bool TcpClient::connected() const {
    std::lock_guard lock(mutex);
    if ( !pimpl )
        throw error::SetupError("TcpClient not connected!");
    return pimpl->connected();
}

size_t TcpClient::bytes_available() const {
    std::lock_guard lock(mutex);
    if ( !pimpl )
        throw error::SetupError("TcpClient not connected!");
    return pimpl->bytes_available();
}

size_t TcpClient::send(const char* buf, size_t size) {
    std::lock_guard lock(mutex);
    if ( !pimpl )
        throw error::SetupError("TcpClient not connected!");
    return pimpl->send(buf, size);
}

size_t TcpClient::send(const std::string& buf) {
    return send(buf.data(), buf.size());
}

size_t TcpClient::receive(char* buf, size_t size, uint64_t timeout) {
    std::lock_guard lock(mutex);
    if ( !pimpl )
        throw error::SetupError("TcpClient not connected!");
    return pimpl->receive(buf, size, timeout);
}

std::string TcpClient::receive(size_t size, uint64_t timeout) {
    std::string buffer(size, '\0');
    size_t r = receive(&buffer[0], buffer.size(), timeout);
    if ( r == buffer.size() )
        return std::move(buffer);
    return buffer.substr(0, r);
}

size_t TcpClient::peek(char* buf, size_t size, uint64_t timeout) {
    std::lock_guard lock(mutex);
    if ( !pimpl )
        throw error::SetupError("TcpClient not connected!");
    return pimpl->peek(buf, size, timeout);
}

std::string TcpClient::peek(size_t size, uint64_t timeout) {
    std::string buffer(size, '\0');
    size_t r = peek(&buffer[0], buffer.size(), timeout);
    if ( r == buffer.size() )
        return std::move(buffer);
    return buffer.substr(0, r);
}
}
