#include <iostream>
#include <cstdlib>
#include <ctime>
#include <memory>

#include "TextualCosmic.hpp"
#include "AlienBase.hpp"
#include "Aliens.hpp"
#include "CosmicServer.hpp"

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " <number of players> [seed]\n";
		return 1;
	}
	unsigned num_players;

	//TODO: Use getopt or descore for parameters
	//TODO: Have the server's listening port be a param here
	if(argc == 2)
	{
		num_players = std::stoi(argv[1]);
		std::srand(unsigned (std::time(0)));
	}
	else
	{
		num_players = std::stoi(argv[1]);
		std::srand(std::stoi(argv[2]));
	}

	std::cout << "Local IP address for server: " << find_local_ip_address() << "\n";

	//TODO: Add a password just for paranoia?
	int listen_port = 3074; //Commonly used port for games
	CosmicServer server(listen_port);

	server.create_listening_socket();
	server.init_server_addr();
	server.bind_listening_socket();
	server.set_linger_opts_for_listening_socket();
	server.listen();
	server.accept_client(PlayerColors::Red);
	server.accept_client(PlayerColors::Blue);
	if(num_players > 2)
		server.accept_client(PlayerColors::Purple);
	if(num_players > 3)
		server.accept_client(PlayerColors::Yellow);
	if(num_players > 4)
		server.accept_client(PlayerColors::Green);
	server.close_listening_socket();

    	std::cout << "Textual Cosmic\n\n";
	GameState game(num_players,server);
    	game.dump();

	game.assign_aliens();
	game.deal_starting_hands();
	game.send_player_hands();
	game.choose_first_player();

	game.start_game();

	server.close_client(PlayerColors::Red);
	server.close_client(PlayerColors::Blue);
	if(num_players > 2)
		server.close_client(PlayerColors::Purple);
	if(num_players > 3)
		server.close_client(PlayerColors::Yellow);
	if(num_players > 4)
		server.close_client(PlayerColors::Green);

    	return 0;
}
