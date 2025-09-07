# MicroHTTP

This project aims to be a simple and fast HTTP micro framework written in C.

## Installation

### Cloning the repository
```sh
git clone https://github.com/AllanDantas21/MicroHTTP.git && cd MicroHTTP
```

### Building the library
```sh
make -f Makefile.lib
```

### Running the example
```sh
./examples/simple_server
```

## Usage

### Basic setup

To create an HTTP server, you need to include the necessary headers and configure the server:

```c
#include "includes/api/httpc.h"
#include "includes/api/http.h"

int main(void) {
    // Initialize the framework
    if (httpc_init() != 0) {
        return 1;
    }
    
    // Configure the server
    httpc_config_t config = {
        .port = 8080,
        .backlog = 10,
        .max_clients = 10,
        .host = "127.0.0.1"
    };
    
    if (httpc_configure(&config) != 0) {
        httpc_cleanup();
        return 1;
    }
    
    // Add routes here
    
    // Start the server
    return httpc_start();
}
```

### Defining routes

Routes are defined using the `httpc_add_route` function. Each route needs an HTTP method, a path, and a handler function:

```c
// Handler for the main route
char* handle_home(const char* buffer) {
    const char* body = "<html><body><h1>Welcome!</h1></body></html>";
    char* response = build_response(200, "text/html", body);
    return response;
}

// Add the route
httpc_add_route(&g_router, "GET", "", handle_home);
```

### Complete example

See the `examples/simple_server.c` file for a complete implementation example with multiple routes and JSON handling.

