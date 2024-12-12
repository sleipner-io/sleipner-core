/**
 * Copyright (c) 2024 Ferdinand Tonby-Strandborg
 * This software is provided under the MIT License.
 * See LICENSE file for details
 */
/**
 * @file winsock.hpp
 * @brief Provides WinsockLoader in order to startup/cleanup winsock in a RAII fashion with the first/last instance
 * @author Ferdinand Tonby-Strandborg
 */
#ifndef _SLEIPNER_SYS_WINSOCK_HPP_
#define _SLEIPNER_SYS_WINSOCK_HPP_

#include <mutex>
#include <atomic>
#include <cstdint>

namespace sleipner::sys {
/**
 * @brief Startup and cleanup WinSock with first/last instance of this class
 *
 * Simple example
 * @code
 * // Winsock will only be loaded in this code-block
 * {
 *  WinsockLoader wsl;
 *  std::cout << "Loaded winsock version: " << wsl.version_major() << "." << wsl.version_minor() << std::endl;
 *  // Prints "Loaded winsock version: x.x" on a Windows device
 *  // Prints "Loaded winsock version: 0.0" on a non-Windows device
 * }
 *
 * // Winsock will be perpetually loaded, as the instance is never destructed, following:
 * new WinsockLoader();
 */
class WinsockLoader {
public:
    /**
     * @brief Increment count and start up WSA if applicable
     *
     * @throws SystemApiError if WSA startup failed
     */
    WinsockLoader() {}

    /**
     * @brief Decrement count and clean up WSA if applicable
     */
    ~WinsockLoader() {}

    /// @brief Copy constructor does nothing
    WinsockLoader(const WinsockLoader&) {}

    /// @brief Copy assignment does nothing
    WinsockLoader& operator=(const WinsockLoader&) { return *this; }

protected:
    /**
     * @brief This is where the magic happens! Don't worry about it ;)
     */
    struct InstanceTracker {
        /**
         * @brief The constructor increments the counter and starts WSA if applicable
         *
         * @throws SystemApiError
         */
        InstanceTracker();

        /**
         * @brief The destructor decrements the counter and clean up WSA if applicable
         */
        ~InstanceTracker();
    } tracker;

    friend InstanceTracker;
        /**
         * @brief Mutex to ensure no data races in starting up and cleaning up WSA
         */
        static std::mutex          mutex;

        /**
         * @brief Atomic instance counter
         */
        static std::atomic<size_t> counter;

public:
    /**
     * @brief Retrieve the count of instances of this class
     */
    static size_t get_count();

    /**
     * @brief Retrieve the major version of WinSock that is loaded
     */
    uint8_t version_major() const;

    /**
     * @brief Retrieve the minor version of WinSock that is loaded
     */
    uint8_t version_minor() const;
};
}

#endif
