/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
/**
 * @file error.hpp
 * @brief Errors specific to the transport components
 * @author Ferdinand Tonby-Strandborg
 */
#ifndef _SLEIPNER_TRANSPORT_ERROR_HPP_
#define _SLEIPNER_TRANSPORT_ERROR_HPP_

#include <stdexcept>
#include <string>

namespace sleipner::error {
/**
 * @brief The socket instance is inappropriately set-up before operations such as send are run.
 */
class SetupError: public std::logic_error {
    public:
        explicit SetupError(const std::string& msg);
};

/**
 * @brief The socket has been externally disconnected or closed.
 */
class SocketDisconnection: public std::runtime_error {
    public:
        explicit SocketDisconnection(const std::string& msg);
};

/**
 * @brief Failed to connect socket despite valid target - could be the target is already occupied.
 */
class ConnectionFailure: public std::runtime_error {
    public:
        explicit ConnectionFailure(const std::string& msg);
};
}

#endif
