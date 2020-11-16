#include <iostream>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <sstream>

#include "cpptk.h"

//Client/Server includes
#if defined(__unix__) || defined(__APPLE__)
	#include <errno.h>
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <string.h>
#elif _WIN32
	#define _WIN32_WINNT 0x501 //Some dependency for ws2tcpip.h? WTF?
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "Ws2_32.lib")
#else
	#error Unexpected platform
#endif

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
	#if defined(__unix__) || defined(__APPLE__)
		unsigned msg_size = message.size()+1;
		int res = write(socket, message.c_str(), msg_size);
		check_error(res,"writing message to server");
	#elif _WIN32
		unsigned msg_size = message.size()+1;
		int res = send(socket, message.c_str(), msg_size, 0); //For successful writes, res will contain the number of bytes sent
		if(res == SOCKET_ERROR)
		{
			std::cerr << "Error sending message: " << WSAGetLastError() << "\n";
			closesocket(socket);
			WSACleanup();
			std::exit(1);
		}
	#endif
}

std::string read_message_from_server(int socket)
{
	#if defined(__unix__) || defined(__APPLE__)
		char buffer[1024];
		int res = read(socket, buffer, 1023);
		check_error(res,"reading message from server");

		//At this point the client has received res bytes from the server
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

		return ret;
	#elif _WIN32
		char buffer[1024];
		int res = recv(socket, buffer, 1023, 0);
		if(res < 0)
		{
			std::cerr << "Receive failed: " << WSAGetLastError() << "\n";
			std::exit(1);
		}

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

		return ret;
	#endif
}

std::string filter_response(const std::string &response)
{
	//NOTE: This filtering is generally dependent on the terminal rather than something we control in C++. I suppose we'll eventually want some sort of drop down menu
	//Left arrow: 27 91 68
	//Right arrow: 27 91 67
	//Up arrow: 27 91 65
	//Down arrow: 27 91 66
	for(unsigned i=0; i<response.size(); i++)
	{
		if(static_cast<int>(response[i]) == 27)
		{
			if((i+2 < response.size()) && (static_cast<int>(response[i+1]) == 91) && (static_cast<int>(response[i+2]) <= 68) && (static_cast<int>(response[i+2]) >= 65))
			{
				//Found an arrow key, get rid of it
				std::string subresponse;
				if(i+3 < response.size())
				{
					subresponse = std::string(response.begin()+i+3,response.end());
				}
				else //Removing it hits the end of the string
				{
					return std::string("");
				}
				return filter_response(subresponse);
			}
		}
	}

	return response;
}

bool parse_command(const std::string &command)
{
	std::cout << "Command: " << command << "\n";
	const std::string &card_delim("card ");
	const std::string &help_delim("help");
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

		return true;
	}
	else if(command.rfind(help_delim,0) == 0) //Provide list of valid commands
	{
		std::stringstream info_str;
		info_str << "Valid player commands:\n";
		info_str << "info help\n";
		info_str << "\tReturns this message.\n";
		info_str << "info board\n";
		info_str << "\tReturns the current game scores, planets, warp, and hyperspace gate.\n";
		info_str << "info hand\n";
		info_str << "\tReturns the contents of the player's hand.\n";
		info_str << "info alien\n";
		info_str << "\tReturns the player's alien power.\n";
		info_str << "info aliens\n";
		info_str << "\tReturns any aliens that have been revealed so far.\n";
		info_str << "info card <Cosmic Card Type>\n";
		info_str << "\tReturns the description of <CosmicCardType>. For example: 'info card ForceField'.\n";

		std::cout << info_str.str() << "\n";

		return true;
	}

	return false;
}

//TODO: Create a GUI out of what we have here
//	+1) Have an initial window where the user enters in the host and port
//	2) Once the user has been connected, use a new window for the game itself
//	3) Initially the game window can have a textbox of server messages (essentially moving client std::cout messages there) and radio buttons for any choices that need to be made
int main(int argc, char *argv[])
{
	Tk::init(argv[0]);

	std::string peer_host("localhost");
	std::string server_port_str("1234");

	auto connect_to_server = [&] ()
	{
		short server_port = std::stoi(server_port_str);

#if defined(__unix__) || defined(__APPLE__)
		//Create socket
		int s0 = socket(AF_INET, SOCK_STREAM, 0);
		check_error(s0,"creating socket");

		//Fill in server IP address
		sockaddr_in server_addr;
		//int server_addr_len;
		memset(&server_addr, '\0', sizeof(server_addr));

		//Resolve server address (convert from symbolic name to IP)
		hostent *host = gethostbyname(peer_host.c_str());
		if(host == nullptr)
		{
			std::cerr << "Error resolving server address: " << strerror(errno) << "\n";
			std::exit(1);
		}

		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(server_port);

		//Write resolved IP address of the server to the address structure
		memmove(&(server_addr.sin_addr.s_addr), host->h_addr_list[0], 4);

		//Connect to the remote server
		int res = connect(s0, (sockaddr*)&server_addr, sizeof(server_addr));
		check_error(res, "connecting to remote server");
#elif _WIN32
		//Initialize Winsock
		WSAData wsaData;
		int res = WSAStartup(MAKEWORD(2,2), &wsaData);
		if(res != 0)
		{
			std::cerr << "WSAStartup failed: " << res << "\n";
			std::exit(1);
		}

		//Create socket
		addrinfo *result = nullptr;
		addrinfo *ptr = nullptr;
		addrinfo hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC; //NOTE: May want to use AF_INET instead here
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		std::stringstream port_stream;
		port_stream << server_port;
		std::string port_str = port_stream.str();
		res = getaddrinfo(peer_host.c_str(),port_str.c_str(),&hints,&result);
		if(res != 0)
		{
			std::cerr << "getaddrinfo failed: " << res << "\n";
			WSACleanup();
			std::exit(1);
		}

		SOCKET s0 = INVALID_SOCKET;

		//Attempt to connect to the first address returned by the call to getaddrinfo
		ptr = result;

		s0 = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if(s0 == INVALID_SOCKET)
		{
			std::cerr << "Error at socket(): " << WSAGetLastError() << "\n";
			freeaddrinfo(result);
			WSACleanup();
			std::exit(1);
		}

		//Connect to the server
		res = connect(s0, ptr->ai_addr, (int)ptr->ai_addrlen);
		if(res == SOCKET_ERROR)
		{
			closesocket(s0);
			s0 = INVALID_SOCKET;
		}

		freeaddrinfo(result);

		if(s0 == INVALID_SOCKET)
		{
			std::cerr << "Unable to connect to server.\n";
			WSACleanup();
			std::exit(1);
		}
#endif

		std::cout << "Connected to server. Reading message...\n";
		Tk::wm(Tk::withdraw, ".w"); //Have the window manager 'forget' about the connection window and remove it from the screen (for some reason attempting to delete the window doesn't work here...maybe we need more event loop calls?)

		//Pop up the new window with server information
		/*Tk::toplevel(".g");
		Tk::wm(Tk::title, ".g", "Textual Cosmic");
		Tk::frame(".g.f");
		Tk::grid(Tk::configure,".g.f") -Tk::column(0) -Tk::row(0);
		//TODO: Add a text box for whatever info we receive from the server
		Tk::textw(".g.f.server_log") -Tk::width(200) -Tk::height(400) -Tk::state(Tk::disabled);
		Tk::grid(Tk::configure,".g.f.server_log") -Tk::column(0) -Tk::row(0) -Tk::padx(5) -Tk::pady(5);*/

		//TODO: Figure out a way to return control to the event loop while the player waits on the server for more information
		//Might need a separate dispatch thread (std::future?) to look for game information

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
				bool local_command_handled = false;
				do
				{
					std::cout << buf << std::endl;
					std::cout << "How would you like to respond?\n";
					std::getline(std::cin,response);

					response = filter_response(response);

					const std::string &command_delimeter("info ");
					if(response.rfind(command_delimeter,0) == 0) //The player responded with a command
					{
						std::string command(response.begin()+command_delimeter.size(),response.end());
						local_command_handled = parse_command(command);
					}
					else
					{
						local_command_handled = false;
					}
				} while(local_command_handled);

				send_message_to_server(s0,response);
			}
			else
			{
				std::cout << buf << std::endl;
			}
		}

#if defined(__unix__) || defined(__APPLE__)
		close(s0);
#elif _WIN32
		closesocket(s0);
		WSACleanup();
#endif
	};

	//Hide the main window, as there's nothing to show there yet
	Tk::wm(Tk::withdraw,".");

	//Create a new window for the connection screen
	Tk::toplevel(".w");
	Tk::wm(Tk::title, ".w", "Textual Cosmic");
	Tk::frame(".w.f");
	Tk::grid(Tk::configure,".w.f") -Tk::column(0) -Tk::row(0);
	Tk::label(".w.f.ip_label") -Tk::text("Server IP address:");
	Tk::grid(Tk::configure,".w.f.ip_label") -Tk::column(0) -Tk::row(0) -Tk::padx(5) -Tk::pady(5);
	Tk::entry(".w.f.ip_entry") -Tk::textvariable(peer_host); //TODO: Could add fancy validation here
	Tk::grid(Tk::configure,".w.f.ip_entry") -Tk::column(1) -Tk::row(0) -Tk::padx(5) -Tk::pady(5);
	Tk::label(".w.f.port_label") -Tk::text("Server Port:");
	Tk::grid(Tk::configure,".w.f.port_label") -Tk::column(0) -Tk::row(1) -Tk::padx(5) -Tk::pady(5);
	Tk::entry(".w.f.port_entry") -Tk::textvariable(server_port_str); //TODO: More validation
	Tk::grid(Tk::configure,".w.f.port_entry") -Tk::column(1) -Tk::row(1) -Tk::padx(5) -Tk::pady(5);
	Tk::button(".w.f.connect") -Tk::text("Connect") -Tk::command(connect_to_server);
	Tk::grid(Tk::configure,".w.f.connect") -Tk::column(0) -Tk::row(2) -Tk::padx(5) -Tk::pady(5) -Tk::columnspan(2);
	Tk::bind(".w", "<Return>", connect_to_server); //Allow the enter key to connect as well

	Tk::runEventLoop();

    	return 0;
}
