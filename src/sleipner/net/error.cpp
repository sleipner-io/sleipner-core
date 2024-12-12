/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
#include "sleipner/net/error.hpp"

namespace sleipner::error {
ResolutionFailure::ResolutionFailure(const std::string& msg): std::domain_error(msg) {}
}
