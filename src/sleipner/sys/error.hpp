/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
/**
 * @file error.hpp
 * @brief Errors related to the core/system services and APIs.
 * @author Ferdinand Tonby-Strandborg
 */
#ifndef _SLEIPNER_SYS_ERROR_HPP_
#define _SLEIPNER_SYS_ERROR_HPP_

#include <system_error>
#include <string>

namespace sleipner::error {
/**
 * @brief The underlying system has failed at handling a valid request.
 */
class SystemApiError: public std::system_error {
    public:
        /**
         * @brief The error-code thrown
         *
         * @note 0 may be a valid error code, depending on the system, or it may be an un-set error code
         */
        int error_code = 0;

        /// @brief Construct with error code and message
        explicit SystemApiError(int code, const std::string& msg);

        /// @brief Error message is retrieved using @b error_message function
        explicit SystemApiError(int code);
};
}

namespace sleipner::sys {
/**
 * @brief Retrieve a human-readable message describing the error code.
 *
 * @param [in] code The error code
 * @return std::string Message describing the error, or an empty string if no message found for the code
 */
std::string error_message(int code) noexcept;
}

#endif
