cmake_minimum_required(VERSION 3.10)
project(httpc VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build tests" OFF)

include_directories(${CMAKE_SOURCE_DIR}/includes)

set(HTTPC_SOURCES
    srcs/httpc_lib.c
    srcs/server/server.c
    srcs/router/router.c
    srcs/routes/routes.c
    srcs/handlers/handler.c
    srcs/handlers/methods/get_handler.c
    srcs/handlers/methods/post_handler.c
    srcs/handlers/methods/unsupported_handler.c
    srcs/logger/logger.c
    srcs/json/cJSON.c
)

add_library(httpc_static STATIC ${HTTPC_SOURCES})
set_target_properties(httpc_static PROPERTIES
    OUTPUT_NAME httpc
    POSITION_INDEPENDENT_CODE ON
)

if(BUILD_SHARED_LIBS)
    add_library(httpc_shared SHARED ${HTTPC_SOURCES})
    set_target_properties(httpc_shared PROPERTIES
        OUTPUT_NAME httpc
        POSITION_INDEPENDENT_CODE ON
    )
endif()

if(BUILD_EXAMPLES)
    add_executable(simple_example examples/simple_server.c)
    target_link_libraries(simple_example httpc_static)
    
    target_link_libraries(original_example httpc_static)
endif()

install(TARGETS httpc_static
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

if(BUILD_SHARED_LIBS)
    install(TARGETS httpc_shared
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

install(DIRECTORY includes/ DESTINATION include/httpc)

# Instalar arquivo pkg-config
configure_file(httpc.pc.in httpc.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/httpc.pc DESTINATION lib/pkgconfig)
