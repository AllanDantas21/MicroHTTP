#include "http.h"

void welcome_message(void)
{
    printf("--------------------------------\n");
    printf("Server is running on port %d\n", PORT);
    printf("Waiting for connections...\n");
    printf("--------------------------------\n");
}
