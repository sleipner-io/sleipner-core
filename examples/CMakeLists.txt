set(EXAMPLE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/transport/get-request.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sys/error-lookup.cpp
)

foreach( EXAMPLE_FILE ${EXAMPLE_SOURCES} )
    get_filename_component(FILE_NAME ${EXAMPLE_FILE} NAME_WE)
    get_filename_component(FULL_DIR ${EXAMPLE_FILE} DIRECTORY)
    get_filename_component(DIR_NAME ${FULL_DIR} NAME)

    set(EXAMPLE_NAME ${DIR_NAME}-${FILE_NAME})
    message(STATUS "sleipner-core --> examples/${EXAMPLE_NAME} [${EXAMPLE_FILE}]")

    add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})

    target_link_libraries(${EXAMPLE_NAME} sleipner::core)
endforeach()
