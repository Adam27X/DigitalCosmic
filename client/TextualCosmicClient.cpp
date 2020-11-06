#include <iostream>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <sstream>

//Client/Server includes
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "CosmicDeck.hpp"

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

void parse_command(const std::string &command)
{
	std::cout << "Command: " << command << "\n";
	const std::string &card_delim("card_");
	if(command.rfind(card_delim,0) == 0) //CosmicCard command, can be handled locally by the client
	{
		std::string info_str;
		std::string cosmic_card(command.begin()+card_delim.size(),command.end());
		if(cosmic_card.compare("Attack") == 0)
		{
			info_str = card_info(CosmicCardType::Attack0); //We use the same description for all attack cards
		}
		else if(cosmic_card.compare("Negotiate") == 0)
		{
			info_str = card_info(CosmicCardType::Negotiate);
		}
		else if(cosmic_card.compare("Morph") == 0)
		{
			info_str = card_info(CosmicCardType::Morph);
		}
		else if(cosmic_card.compare("Reinforcement") == 0)
		{
			info_str = card_info(CosmicCardType::Reinforcement2);
		}
		else if(cosmic_card.compare("CardZap") == 0)
		{
			info_str = card_info(CosmicCardType::CardZap);
		}
		else if(cosmic_card.compare("CosmicZap") == 0)
		{
			info_str = card_info(CosmicCardType::CosmicZap);
		}
		else if(cosmic_card.compare("MobiusTubes") == 0)
		{
			info_str = card_info(CosmicCardType::MobiusTubes);
		}
		else if(cosmic_card.compare("Plague") == 0)
		{
			info_str = card_info(CosmicCardType::Plague);
		}
		else if(cosmic_card.compare("ForceField") == 0)
		{
			info_str = card_info(CosmicCardType::ForceField);
		}
		else if(cosmic_card.compare("EmotionControl") == 0)
		{
			info_str = card_info(CosmicCardType::EmotionControl);
		}
		else if(cosmic_card.compare("Quash") == 0)
		{
			info_str = card_info(CosmicCardType::Quash);
		}
		else if(cosmic_card.compare("IonicGas") == 0)
		{
			info_str = card_info(CosmicCardType::IonicGas);
		}
		else
		{
			std::stringstream ret;
			ret << "Error: Unknown card type. Here are the valid options to this command:\n";
			ret << "Attack\n";
			ret << "Negotiate\n";
			ret << "Morph\n";
			ret << "Reinforcement\n";
			ret << "CardZap\n";
			ret << "CosmicZap\n";
			ret << "MobiusTubes\n";
			ret << "Plague\n";
			ret << "ForceField\n";
			ret << "EmotionControl\n";
			ret << "Quash\n";
			ret << "IonicGas\n";
			ret << "Please choose one of the above options.\n";
			std::cout << ret.str();
		}

		std::cout << info_str << "\n";
	}
	else
	{
		std::cout << "Invalid command.\n";
	}
}

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
			bool command_sent = false;
			do
			{
				std::cout << buf << "\n";
				std::cout << "How would you like to respond?\n";
				std::cin >> response;

				const std::string &command_delimeter("info_");
				if(response.rfind(command_delimeter,0) == 0) //The player responded with a command
				{
					command_sent = true;
					std::string command(response.begin()+command_delimeter.size(),response.end());
					parse_command(command);
				}
				else
				{
					command_sent = false;
				}
			} while(command_sent);

			send_message_to_server(s0,response);
		}
		else
		{
			std::cout << buf << "\n";
		}
	}

	close(s0);

    	return 0;
}
