/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
#include "sleipner/transport/error.hpp"

namespace sleipner::error {
SetupError::SetupError(const std::string& msg): std::logic_error(msg) {}

SocketDisconnection::SocketDisconnection(const std::string& msg): std::runtime_error(msg) {}

ConnectionFailure::ConnectionFailure(const std::string& msg): std::runtime_error(msg) {}
}
