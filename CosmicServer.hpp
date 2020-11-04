#pragma once

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include <string>
#include <map>

#include "PlayerColors.hpp"

std::string find_local_ip_address();

class CosmicServer
{
public:
	CosmicServer(int listen_port) : m_listen_port(listen_port) { }

	void create_listening_socket();
	void init_server_addr();
	void bind_listening_socket();
	void set_linger_opts_for_listening_socket();
	void listen();
	void accept_client();
	void close_listening_socket();
	void send_message_to_client(const std::string &message);
	std::string receive_message_from_client();
	void close_client();
	std::string find_local_ip_address();

private:
	void check_error(int res, const std::string &context) const;
	int m_listen_port;
	int m_listen_socket;
	sockaddr_in m_server_addr;
	sockaddr_in m_client_addr;
	std::map<PlayerColors,int> m_client_socket_map;
};
