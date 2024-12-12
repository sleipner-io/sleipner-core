/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
#include "sleipner/net/ip.hpp"
#include "sleipner/sys/error.hpp"
#include "sleipner/net/error.hpp"

#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>

    #include "sleipner/sys/winsock.hpp"
#else
    #error "ip.cpp yet to be implemented..."
#endif

namespace sleipner::net {
    static IpAddress translate_addrinfo(const ::addrinfo& info) {
        IpAddress new_addr{};

        new_addr.family = info.ai_family;

        // If this is invalid, delay error...
        if ( info.ai_addr )
            new_addr.addr = {reinterpret_cast<char*>(const_cast<::sockaddr*>(info.ai_addr)), info.ai_addrlen};

        return new_addr;
    }

    static ::sockaddr_in* expose_in(const IpAddress& address) {
        if ( address.addr.size() < sizeof(sockaddr_in) )
            throw std::invalid_argument("Malformed address!");
        return reinterpret_cast<::sockaddr_in*>(const_cast<char*>(address.addr.data()));
    }

    static ::sockaddr_in6* expose_in6(const IpAddress& address) {
        if ( address.addr.size() < sizeof(sockaddr_in6) )
            throw std::invalid_argument("Malformed address!");
        return reinterpret_cast<::sockaddr_in6*>(const_cast<char*>(address.addr.data()));
    }

    std::vector<IpAddress> resolve_ip(const std::string& hostname, uint16_t port) {
        #ifdef _WIN32
            sys::WinsockLoader wsl;
        #endif

        ::addrinfo hints {0};
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = 0;

        if ( hostname.empty() || port == 0 )
            throw std::invalid_argument("Hostname can't be empty, and port can't be 0!");

        std::string port_str = std::to_string(port);

        ::addrinfo* resolved = nullptr;

        int err = ::getaddrinfo(hostname.c_str(), port_str.c_str(), &hints, &resolved);

        if ( err ) {
            err = ::WSAGetLastError();

            switch ( err ) {
                case WSAHOST_NOT_FOUND:
                case WSANO_DATA:
                case WSATRY_AGAIN:   // Consider having separate exception for TRY_AGAIN and NO_RECOVERY
                case WSANO_RECOVERY:
                    throw error::ResolutionFailure(sys::error_message(err));

                // case WSATYPE_NOT_FOUND:
                // case WSAESOCKTNOSUPPORT:
                // case WSAEINVAL:
                // case WSAEAFNOSUPPORT:
                // case WSA_NOT_ENOUGH_MEMORY:
                // case WSANOTINITIALIZED:
                default:
                    throw error::SystemApiError(err);
            }
        }

        std::vector<IpAddress> addresses;

        // Though all should be AF_INET or AF_INET6, check just in case of an unexpected result...
        for ( ::addrinfo* curr = resolved; curr; curr = curr->ai_next )
            if ( curr->ai_family == AF_INET || curr->ai_family == AF_INET6 )
                addresses.push_back(translate_addrinfo(*curr));

        ::freeaddrinfo(resolved);
        return addresses;
    }

    std::string get_ip_address(const IpAddress& target) {
        char addr[INET6_ADDRSTRLEN]; // INET 6 longer than INET
        if ( target.family == AF_INET && inet_ntop(AF_INET, &expose_in(target)->sin_addr, addr, sizeof(addr)) )
            return addr;
        else if ( target.family == AF_INET6 && inet_ntop(AF_INET6, &expose_in6(target)->sin6_addr, addr, sizeof(addr)) )
            return addr;
        else
            throw std::invalid_argument("Invalid family!");
    }

    uint16_t get_port(const IpAddress& target) {
        if ( target.family == AF_INET )
            return ntohs(expose_in(target)->sin_port);
        else if ( target.family == AF_INET6 )
            return ntohs(expose_in6(target)->sin6_port);
        throw std::invalid_argument("Invalid family!");
    }
}
