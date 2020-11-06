#include <iostream>
#include <cstdlib>
#include <sstream>

#include "CosmicServer.hpp"

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
	m_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	check_error(m_listen_socket,"creating socket");
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
	check_error(res, "binding socket");
}

void CosmicServer::set_linger_opts_for_listening_socket()
{
	//Setting the linger timeout to 0 closes the listen socket immediately at program termination
	//TODO: Do we want this behavior or can we close the listen socket once all players have connected?
	linger linger_opts = {1, 0}; //Linger active, timeout 0
	setsockopt(m_listen_socket, SOL_SOCKET, SO_LINGER, &linger_opts, sizeof(linger_opts));
}

void CosmicServer::listen()
{
	int res = ::listen(m_listen_socket, 1); //1 is the maximal length of the queue, so we only accept one connection?
	check_error(res,"listening");
}

void CosmicServer::accept_client(const PlayerColors color)
{
	socklen_t client_addr_len; //Not sure what we do with this field...
	int client_socket = accept(m_listen_socket, (sockaddr*)&m_client_addr, &client_addr_len);
	check_error(client_socket,"accepting connection");
	m_client_socket_map.insert(std::pair<PlayerColors,int>(color,client_socket));

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
	int res = close(m_listen_socket);
	check_error(res,"closing listening socket");
}

void CosmicServer::send_message_to_client(const PlayerColors color, const std::string &message)
{
	unsigned msg_size = message.size()+1;
	int res = write(m_client_socket_map[color], message.c_str(), msg_size);
	check_error(res,"writing message to client");
}

//Send to all clients
void CosmicServer::broadcast_message(const std::string &message)
{
	unsigned msg_size = message.size()+1;
	for(auto i=m_client_socket_map.begin(),e=m_client_socket_map.end();i!=e;++i)
	{
		int res = write(i->second, message.c_str(), msg_size);
		check_error(res,"writing message to client");
	}
}

std::string CosmicServer::receive_message_from_client(const PlayerColors color)
{
	//TODO: How can we tell if the message is complete? Technically clients only need to send a byte at a time, but still
	char buffer[1024];
	int res = read(m_client_socket_map[color], buffer, 1023);
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
	buffer[res] = 0;
	std::string ret;
	ret = std::string(buffer,res-1);
	std::cout << "Received " << res << " bytes from client.\n";
	return ret;
}

void CosmicServer::close_client(const PlayerColors color)
{
	int res = close(m_client_socket_map[color]);
	check_error(res,"closing client socket");
	m_client_socket_map.erase(color);
}

std::string find_local_ip_address()
{
        //Apparently the most reliable way to get your own IP is to ask a remote endpoint such as Google
	const char* google_dns_server = "8.8.8.8";
	int dns_port = 53;
	sockaddr_in google_addr;
	int google_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(google_socket < 0)
	{
		std::cerr << "Error creating Google socket: " << strerror(errno) << "\n";
		std::exit(1);
	}
	memset(&google_addr, 0, sizeof(google_addr));
	google_addr.sin_family = AF_INET;
	google_addr.sin_port = htons(dns_port);
	google_addr.sin_addr.s_addr = inet_addr(google_dns_server);
	int res = connect(google_socket, (sockaddr*)&google_addr, sizeof(google_addr));
	if(res < 0)
	{
		std::cerr << "Error connecting to Google DNS server: " << strerror(errno) << "\n";
	}
	sockaddr_in name;
	socklen_t namelen = sizeof(name);
	res = getsockname(google_socket, (sockaddr*)&name, &namelen);
	char buffer[80];
	const char *p = inet_ntop(AF_INET, &name.sin_addr, buffer, 80);
	if(!p)
	{
		std::cerr << "Error resolving local IP: " << strerror(errno) << "\n";
	}
	close(google_socket);
	return std::string(buffer);
}

