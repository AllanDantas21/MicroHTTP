cmake_minimum_required(VERSION 3.10)
project(httpc VERSION 1.0.0 LANGUAGES C)

# Configurações do compilador
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Opções de compilação
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build tests" OFF)

# Incluir diretórios
include_directories(${CMAKE_SOURCE_DIR}/includes)

# Arquivos fonte da biblioteca
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
)

# Criar biblioteca estática
add_library(httpc_static STATIC ${HTTPC_SOURCES})
set_target_properties(httpc_static PROPERTIES
    OUTPUT_NAME httpc
    POSITION_INDEPENDENT_CODE ON
)

# Criar biblioteca compartilhada
if(BUILD_SHARED_LIBS)
    add_library(httpc_shared SHARED ${HTTPC_SOURCES})
    set_target_properties(httpc_shared PROPERTIES
        OUTPUT_NAME httpc
        POSITION_INDEPENDENT_CODE ON
    )
endif()

# Exemplos de aplicação
if(BUILD_EXAMPLES)
    add_executable(simple_example examples/simple_server.c)
    target_link_libraries(simple_example httpc_static)
    
    # Exemplo original
    add_executable(original_example srcs/http.c)
    target_link_libraries(original_example httpc_static)
endif()

# Instalação
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
