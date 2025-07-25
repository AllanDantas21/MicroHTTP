#include "http.h"

void logger_message(logger_status status)
{
    switch (status) {
        case LOG_WELCOME:
            welcome_message();
            break;
        case LOG_REQUEST:
            break;
        case LOG_ERROR:
            break;
        case LOG_CONNECTION:
            break;
    }
}
