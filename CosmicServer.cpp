#include <iostream>
#include <cstdlib>
#include <sstream>

#include "CosmicServer.hpp"

CosmicServer::CosmicServer(int listen_port) : m_listen_port(listen_port)
{
#ifdef _WIN32
	//Initialize Winsock
	WSAData wsaData;
	int res = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(res != 0)
	{
		std::cerr << "WSAStartup failed: " << res << "\n";
		std::exit(1);
	}
#endif
}

void CosmicServer::check_error(int res, const std::string &context) const
{
	if(res < 0)
	{
		std::cerr << "Error " << context << ": " << strerror(errno) << "\n";
		std::exit(1);
	}
}

void CosmicServer::create_listening_socket()
{
#ifdef _WIN32
	m_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_listen_socket == INVALID_SOCKET)
	{
		std::cerr << "Socket failed with error " << WSAGetLastError() << "\n";
		WSACleanup();
		std::exit(1);
	}
#else
	m_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	check_error(m_listen_socket,"creating socket");
#endif
}

void CosmicServer::init_server_addr()
{
	memset(&m_server_addr,0,sizeof(m_server_addr));
	m_server_addr.sin_family = AF_INET;
	m_server_addr.sin_port = htons(m_listen_port);
	m_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//At this point we've accepted a connection
	std::cout << "Hosting server from "
		  << ((ntohl(m_server_addr.sin_addr.s_addr) >> 24) & 0xff) << "."
		  << ((ntohl(m_server_addr.sin_addr.s_addr) >> 16) & 0xff) << "."
		  << ((ntohl(m_server_addr.sin_addr.s_addr) >> 8) & 0xff) << "."
		  << (ntohl(m_server_addr.sin_addr.s_addr) & 0xff) << ", port " << ntohs(m_server_addr.sin_port) << "\n";
}

void CosmicServer::bind_listening_socket()
{
	int res = bind(m_listen_socket, (sockaddr*)&m_server_addr, sizeof(m_server_addr));
#ifdef _WIN32
	if(res == SOCKET_ERROR)
	{
		std::cerr << "Bind failed with error: " << WSAGetLastError() << "\n";
		closesocket(m_listen_socket);
		WSACleanup();
		std::exit(1);
	}
#else
	check_error(res, "binding socket");
#endif
}

void CosmicServer::set_linger_opts_for_listening_socket()
{
	linger linger_opts = {1, 0}; //Linger active, timeout 0

#ifdef _WIN32
	int res = setsockopt(m_listen_socket, SOL_SOCKET, SO_LINGER, (char*) &linger_opts, sizeof(linger_opts));
	if(res == SOCKET_ERROR)
	{
		std::cerr << "Error setting linger socket options: " << WSAGetLastError() << "\n";
		closesocket(m_listen_socket);
		WSACleanup();
		std::exit(1);
	}
#else
	//Setting the linger timeout to 0 closes the listen socket immediately at program termination
	//TODO: Do we want this behavior or can we close the listen socket once all players have connected?
	setsockopt(m_listen_socket, SOL_SOCKET, SO_LINGER, &linger_opts, sizeof(linger_opts));
#endif
}

void CosmicServer::listen()
{
#ifdef _WIN32
	int res = ::listen(m_listen_socket, 1);
	if(res == SOCKET_ERROR)
	{
		std::cerr << "Listen failed with error: " << WSAGetLastError() << "\n";
		closesocket(m_listen_socket);
		WSACleanup();
		std::exit(1);
	}
#else
	int res = ::listen(m_listen_socket, 1); //1 is the maximal length of the queue, so we only accept one connection?
	check_error(res,"listening");
#endif
}

void CosmicServer::accept_client(const PlayerColors color)
{
#ifdef _WIN32
	int client_addr_len = sizeof(m_client_addr);
	SOCKET client_socket = accept(m_listen_socket, (sockaddr*)&m_client_addr, &client_addr_len);
	if(client_socket == INVALID_SOCKET)
	{
		std::cerr << "Accept failed with error: " << WSAGetLastError() << "\n";
		closesocket(m_listen_socket);
		WSACleanup();
		std::exit(1);
	}
#else
	socklen_t client_addr_len;
	int client_socket = accept(m_listen_socket, (sockaddr*)&m_client_addr, &client_addr_len);
	check_error(client_socket,"accepting connection");
#endif
	m_client_socket_map.insert(std::make_pair(color,client_socket));

	//At this point we've accepted a connection
	std::cout << "Accepted connection from "
		  << ((ntohl(m_client_addr.sin_addr.s_addr) >> 24) & 0xff) << "."
		  << ((ntohl(m_client_addr.sin_addr.s_addr) >> 16) & 0xff) << "."
		  << ((ntohl(m_client_addr.sin_addr.s_addr) >> 8) & 0xff) << "."
		  << (ntohl(m_client_addr.sin_addr.s_addr) & 0xff) << ", port " << ntohs(m_client_addr.sin_port) << "\n";

	std::stringstream announce;
	announce << "Welcome to the game. You have been assigned the " << to_string(color) << " player color.\n";
	send_message_to_client(color,announce.str());
}

void CosmicServer::close_listening_socket()
{
#ifdef _WIN32
	int res = closesocket(m_listen_socket);
	if(res == SOCKET_ERROR)
	{
		std::cerr << "Error closing listening socket: " << WSAGetLastError() << "\n";
		WSACleanup();
		std::exit(1);
	}
#else
	int res = close(m_listen_socket);
	check_error(res,"closing listening socket");
#endif
}

void CosmicServer::send_message_to_client(const PlayerColors color, const std::string &message)
{
	if(message.empty())
	{
		return;
	}
	std::string msg_with_sentinel(message);
	msg_with_sentinel.append("END_MESSAGE\n"); //Add the sentinel
	unsigned msg_size = msg_with_sentinel.size()+1;

#ifdef _WIN32
	int res = send(m_client_socket_map[color], msg_with_sentinel.c_str(), msg_size, 0);
	if(res == SOCKET_ERROR)
	{
		std::cerr << "Send failed with error " << WSAGetLastError() << "\n";
		closesocket(m_client_socket_map[color]);
		WSACleanup();
		std::exit(1);
	}
#else
	int res = write(m_client_socket_map[color], msg_with_sentinel.c_str(), msg_size);
	check_error(res,"writing message to client");
#endif
}

//Send to all clients
void CosmicServer::broadcast_message(const std::string &message) const
{
	if(message.empty())
	{
		return;
	}
	std::string msg_with_sentinel(message);
	msg_with_sentinel.append("END_MESSAGE\n"); //Add the sentinel
	unsigned msg_size = msg_with_sentinel.size()+1;
	for(auto i=m_client_socket_map.begin(),e=m_client_socket_map.end();i!=e;++i)
	{
#ifdef _WIN32
		int res = send(i->second, msg_with_sentinel.c_str(), msg_size, 0);
		if(res == SOCKET_ERROR)
		{
			std::cerr << "Send failed with error " << WSAGetLastError() << "\n";
			closesocket(i->second);
			WSACleanup();
			std::exit(1);
		}
#else
		int res = write(i->second, msg_with_sentinel.c_str(), msg_size);
		check_error(res,"writing message to client");
#endif
	}
}

std::string CosmicServer::receive_message_from_client(const PlayerColors color)
{
	char buffer[1024];
#ifdef _WIN32
	int res = recv(m_client_socket_map[color], buffer, 1024, 0);
	if(res <= 0)
	{
		std::cerr << "Recv failed with error " << WSAGetLastError() << "\n";
		closesocket(m_client_socket_map[color]);
		WSACleanup();
		std::exit(1);
	}
	if(res > 1023)
	{
		std::cerr << "Error: Message from client was too long!\n";
		closesocket(m_client_socket_map[color]);
		WSACleanup();
		std::exit(1);
	}
#else
	int res = read(m_client_socket_map[color], buffer, 1024);
	check_error(res,"reading message from client");
	if(res == 0)
	{
		std::cerr << "Error reading message from client\n";
		std::exit(1);
	}
	if(res > 1023)
	{
		std::cerr << "Error: Message from client was too long!\n";
		std::exit(1);
	}
#endif
	buffer[res] = 0;
	std::string ret;
	ret = std::string(buffer,res); //NOTE: For the C++ client, we needed to use 'res-1' here, ugh. Instead we should use 'res' but check the last character for a null-terminating string and act accordingly. For now we'll assume a python client
	std::cout << "Received " << res << " bytes from client.\n";
	return ret;
}

void CosmicServer::close_client(const PlayerColors color)
{
#ifdef _WIN32
	int res = shutdown(m_client_socket_map[color], SD_SEND);
	if(res == SOCKET_ERROR)
	{
		std::cerr << "Shutdown failed with error: " << WSAGetLastError() << "\n";
		closesocket(m_client_socket_map[color]);
		WSACleanup();
		std::exit(1);
	}

	closesocket(m_client_socket_map[color]);
#else
	int res = close(m_client_socket_map[color]);
	check_error(res,"closing client socket");
	m_client_socket_map.erase(color);
#endif
}

std::string find_local_ip_address()
{
        //Apparently the most reliable way to get your own IP is to ask a remote endpoint such as Google
	const char* google_dns_server = "8.8.8.8";
	int dns_port = 53;
	sockaddr_in google_addr;

#ifdef _WIN32
	SOCKET google_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(google_socket == INVALID_SOCKET)
	{
		std::cerr << "Error creating Google socket: " << WSAGetLastError() << "\n";
		WSACleanup();
		std::exit(1);	
	}
#else
	int google_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(google_socket < 0)
	{
		std::cerr << "Error creating Google socket: " << strerror(errno) << "\n";
		std::exit(1);
	}
#endif
	memset(&google_addr, 0, sizeof(google_addr));
	google_addr.sin_family = AF_INET;
	google_addr.sin_port = htons(dns_port);
	google_addr.sin_addr.s_addr = inet_addr(google_dns_server);

	int res = connect(google_socket, (sockaddr*)&google_addr, sizeof(google_addr));
#ifdef _WIN32
	if(res == SOCKET_ERROR)
	{
		std::cerr << "Error connecting to Google DNS server: " << WSAGetLastError() << "\n";
		closesocket(google_socket);
		WSACleanup();
		std::exit(1);
	}
#else
	if(res < 0)
	{
		std::cerr << "Error connecting to Google DNS server: " << strerror(errno) << "\n";
	}
#endif
	sockaddr_in name;
	socklen_t namelen = sizeof(name);
	res = getsockname(google_socket, (sockaddr*)&name, &namelen);
	char buffer[80];

#ifdef _WIN32
	long unsigned int bufferlen = 80;
	res = WSAAddressToString((sockaddr*)&name, namelen, NULL, buffer, &bufferlen);
	if(res == SOCKET_ERROR)
	{
		std::cerr << "Error resolving local IP: " << WSAGetLastError() << "\n";
	}
#else
	const char *p = inet_ntop(AF_INET, &name.sin_addr, buffer, 80);
	if(!p)
	{
		std::cerr << "Error resolving local IP: " << strerror(errno) << "\n";
	}
#endif

#ifdef _WIN32
	closesocket(google_socket);
#else
	close(google_socket);
#endif

	return std::string(buffer);
}

