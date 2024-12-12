cmake_minimum_required(VERSION 3.12)
project(sleipner VERSION 0.0.0)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Define source files relative to src directory
set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/sys/error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/sys/winsock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/net/error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/net/ip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/transport/error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/transport/tcp.cpp
)

set(CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/sys/error.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/sys/winsock.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/net/error.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/net/ip.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/transport/error.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/transport/isocket.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sleipner/transport/tcpclient.hpp
)

add_library(sleipner_core ${CORE_SOURCES} ${CORE_HEADERS})
add_library(sleipner::core ALIAS sleipner_core)

if ( WIN32 )
    target_link_libraries(sleipner_core PRIVATE ws2_32 iphlpapi setupapi)
endif()

set_target_properties(sleipner_core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME core
    OUTPUT_NAME sleipner_core
)

# Correct the include directories
target_include_directories(sleipner_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(sleipner_core
    PUBLIC
        cxx_std_17
)

# Install the library binary
install(TARGETS sleipner_core
    EXPORT sleipner-core-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install the headers maintaining the directory structure
install(
    DIRECTORY src/sleipner
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

# Install the export targets (only once)
install(
    EXPORT sleipner-core-targets
    FILE sleipner-core-targets.cmake
    NAMESPACE sleipner::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sleipner
)

# Generate and install package config files
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sleipner-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sleipner-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sleipner
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/sleipner-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/sleipner-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/sleipner-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sleipner
)



set(BUILD_EXAMPLES ON CACHE BOOL "Build examples")
if ( BUILD_EXAMPLES )
    message(STATUS "sleipner-core --> Building examples...")
    add_subdirectory(examples)
endif()

set(BUILD_TESTS OFF CACHE BOOL "Build tests")
if ( BUILD_TESTS )
    add_subdirectory(tests)
endif()

if ( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/priv.cpp")
    add_executable(priv ${CMAKE_CURRENT_SOURCE_DIR}/priv.cpp)
    target_link_libraries(priv PRIVATE sleipner_core)
    target_include_directories(priv PRIVATE src)
endif()
