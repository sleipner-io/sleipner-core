/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
#include "sleipner/sys/error.hpp"

#ifdef _WIN32
    #include <Windows.h>
#else
    #error "system/error.cpp Not yet implemented..."
#endif

namespace sleipner::error {
SystemApiError::SystemApiError(int code, const std::string& msg):
        std::system_error(std::error_code(code, std::system_category()), msg) {}

SystemApiError::SystemApiError(int code):
        SystemApiError(code, sys::error_message(code)) {}
}

namespace sleipner::sys {
std::string error_message(int code) noexcept {
    #ifdef _WIN32
        char* buffer = nullptr;

        DWORD res = ::FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&buffer),
            0,
            nullptr
        );

        if ( !res || buffer == nullptr )
            return "(" + std::to_string(code) + "):";

        std::string err_msg(buffer);

        ::LocalFree(buffer);

        return "(" + std::to_string(code) + "): " + err_msg;
    #else
        char buffer[256];

        #if defined(__GLIBC__)
            char* err = strerror_r(code, buffer, sizeof(buffer));
        #elif defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L
            char* err = nullptr;
            if ( !strerror_r(code, buffer, sizeof(buffer)) )
                err = buffer;
        #else
            char* err = strerror(code);
        #endif
        return err ? err : "";
    #endif
}
}
