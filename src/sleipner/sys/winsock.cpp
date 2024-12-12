/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
#include "sleipner/sys/winsock.hpp"
#include "sleipner/sys/error.hpp"

#ifdef _WIN32
#include <WinSock2.h>
#endif

namespace sleipner::sys {
#ifdef _WIN32
    static WSADATA _wsadata = {0};
#endif
std::atomic<size_t> WinsockLoader::counter = {0};
std::mutex WinsockLoader::mutex;

WinsockLoader::InstanceTracker::InstanceTracker() {
    if ( WinsockLoader::counter.fetch_add(1) == 0 ) {
            #ifdef _WIN32
                std::lock_guard lock(mutex);
                int err = ::WSAStartup(MAKEWORD(2, 2), &_wsadata);
                if ( err ) {
                    WinsockLoader::counter.fetch_sub(1);
                    throw error::SystemApiError(GetLastError(), "Winsock Startup Failed!");
                }
            #endif
        }
}

WinsockLoader::InstanceTracker::~InstanceTracker() {
    if ( WinsockLoader::counter.fetch_sub(1) == 1 ) {
        #ifdef _WIN32
            std::lock_guard lock(mutex);
            ::WSACleanup();
            _wsadata = {0};
        #endif
    }
}

size_t WinsockLoader::get_count() {
    return counter.load();
}

uint8_t WinsockLoader::version_major() const {
    #ifdef _WIN32
        std::lock_guard lock(mutex);
        return _wsadata.wVersion >> 8;
    #else
        return 0;
    #endif
}

uint8_t WinsockLoader::version_minor() const {
    #ifdef _WIN32
        std::lock_guard lock(mutex);
        return _wsadata.wVersion & 0xFF;
    #else
        return 0;
    #endif
}
}
