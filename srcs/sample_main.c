#include "../includes/http.h"
#include "../includes/server/server.h"
#include "../includes/router.h"

int main(void)
{
	if (init_logger_system() < 0) {
		return 1;
	}

	struct sockaddr_in serverAddress;
	int serverSocket;
	
	setup_server_address(&serverAddress, PORT);
	serverSocket = create_server_socket();
	
	if (bind_server_socket(serverSocket, &serverAddress) != 0) {
		cleanup_logger_system();
		return 1;
	}
	if (start_listening(serverSocket, BACKLOG) != 0) {
		cleanup_logger_system();
		return 1;
	}

	welcome_message();
	main_handler(serverSocket);
	
	cleanup_logger_system();
	return 0;
}
