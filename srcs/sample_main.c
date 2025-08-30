#include "../includes/http.h"
#include "../includes/server/server.h"
#include "../includes/router.h"

int main(void)
{
	struct sockaddr_in serverAddress;
	int serverSocket;
	
	setup_server_address(&serverAddress, PORT);
	serverSocket = create_server_socket();
	
	if (bind_server_socket(serverSocket, &serverAddress) != 0) return 1;
	if (start_listening(serverSocket, BACKLOG) != 0) return 1;

	welcome_message();
	main_handler(serverSocket);
	return 0;
}
