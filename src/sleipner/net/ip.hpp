/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
/**
 * @file ip.hpp
 * @brief Implements functions and structures to resolve IP addresses
 * @author Ferdinand Tonby-Strandborg
 *
 */
#ifndef _SLEIPNER_NET_IP_HPP_
#define _SLEIPNER_NET_IP_HPP_

#include <string>
#include <vector>
#include <cstdint>

namespace sleipner::net {
    /**
     * @brief Used to simplify working with the system-specific socket addresses
     *
     * @warning You should never directly work with this, use instead the methods provided in @b ip.hpp
     */
    struct IpAddress {
        int family     = 0;
        // int sockettype = 0; // Alwasy SOCK_STREAM
        // int protocol   = 0; // Always 0
        std::string addr;
        // std::string canonical_name; // Never referenced...
    };

    /**
     * @brief Resolve the desired IP address from the hostname and port
     *
     * @param [in] hostname Target host (DNS name or IP address)
     * @param [in] port Target port number
     * @throw std::invalid_argument if hostname is empty or port is 0
     * @throws ResolutionFailure
     * @throws SystemApiError
     * @returns A list of resolved addresses
     */
    std::vector<IpAddress> resolve_ip(const std::string& hostname, uint16_t port);

    /**
     * @brief Get the IP address of the target
     *
     * @param [in] target Address to retrieve IP address of
     * @throws std::invalid_argument if the input is malformed
     * @throws SystemApiError
     * @returns String containing the IP address
     */
    std::string get_ip_address(const IpAddress& target);

    /**
     * @brief Return the port of the target
     *
     * @param [in] target Address to retrieve the port of
     * @throws std::invalid_argument if the input is malformed
     * @throws SystemApiError
     * @returns The port
     */
    uint16_t get_port(const IpAddress& target);
}

#endif
