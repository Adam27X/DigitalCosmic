#include <iostream>
#include <cstdlib>
#include <ctime>
#include <memory>

//Client/Server includes
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

void check_error(int res, const std::string &context)
{
	if(res < 0)
	{
		std::cerr << "Error: " << context << ": " << strerror(errno) << "\n";
		std::exit(1);
	}
}

void send_message_to_server(int socket, const std::string &message)
{
	unsigned msg_size = message.size()+1;
	int res = write(socket, message.c_str(), msg_size);
	check_error(res,"writing message to server");
}

std::string read_message_from_server(int socket)
{
	char buffer[1024];
	int res = read(socket, buffer, 1023);
	check_error(res,"reading message from server");
	if(res > 1023)
	{
		std::cerr << "Error: Message from server was too long!\n";
		std::exit(1);
	}
	buffer[res] = 0;
	std::string ret;
	if(res > 0)
	{
		ret = std::string(buffer,res-1);
	}
	else
	{
		ret = std::string();
	}
	std::cout << "Received " << res << " bytes from server.\n";
	return ret;
}

//TODO: Support commands for players coming up with responses; commands that don't require state specific to the current game can be handled locally in this program

int main(int argc, char *argv[])
{
	//Create socket
	int s0 = socket(AF_INET, SOCK_STREAM, 0);
	check_error(s0,"creating socket");

	//Fill in server IP address
	sockaddr_in server_addr;
	//int server_addr_len;
	memset(&server_addr, '\0', sizeof(server_addr));

	std::string peer_host("localhost");
	if(argc > 1)
	{
		peer_host = argv[1];
	}

	//Resolve server address (convert from symbolic name to IP)
	hostent *host = gethostbyname(peer_host.c_str());
	if(host == nullptr)
	{
		std::cerr << "Error resolving server address: " << strerror(errno) << "\n";
		std::exit(1);
	}

	server_addr.sin_family = AF_INET;
	short server_port = 1234;
	if(argc >= 3)
	{
		server_port = (short) atoi(argv[2]);
	}
	server_addr.sin_port = htons(server_port);

	//Write resolved IP address of the server to the address structure
	memmove(&(server_addr.sin_addr.s_addr), host->h_addr_list[0], 4);

	//Connect to the remote server
	int res = connect(s0, (sockaddr*)&server_addr, sizeof(server_addr));
	check_error(res, "connecting to remote server");

	std::cout << "Connected to server. Reading message...\n";

	//TODO: Have the server send some sort of END message once we're done?
	while(1)
	{
		std::string buf = read_message_from_server(s0);
		std::cout << buf << "\n";

		bool needs_response = false;
		if(buf.compare("END") == 0)
		{
			break;
		}
		else if(buf.find("[needs_response]") != std::string::npos)
		{
			needs_response = true;
		}

		if(needs_response)
		{
			std::string response;
			std::cout << "How would you like to respond?\n";
			std::cin >> response;

			send_message_to_server(s0,response);
		}
	}

	close(s0);

    	return 0;
}
