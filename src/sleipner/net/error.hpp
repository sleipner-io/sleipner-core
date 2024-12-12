/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
/**
 * @file error.hpp
 * @brief Errors specific to networking concepts, such as address resolution
 * @author Ferdinand Tonby-Strandborg
 */
#ifndef _SLEIPNER_NET_ERROR_HPP_
#define _SLEIPNER_NET_ERROR_HPP_

#include <stdexcept>
#include <string>

namespace sleipner::error {
/**
 * @brief Failed to resolve the address of a socket or similar
 */
class ResolutionFailure: public std::domain_error {
    public:
        explicit ResolutionFailure(const std::string& msg);
};
}

#endif
