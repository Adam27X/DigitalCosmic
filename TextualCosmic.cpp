#include <iostream>
#include <cstdlib>
#include <ctime>
#include <memory>

#include "TextualCosmic.hpp"
#include "AlienBase.hpp"
#include "Aliens.hpp"
#include "CosmicServer.hpp"

#include <tclap/CmdLine.h>

int main(int argc, char *argv[])
{
	unsigned num_players = 0;
	bool full_control = false;
	unsigned winning_score = 5;
	int listen_port = 3074;

	try
	{
		std::string version = "0.1";
		TCLAP::CmdLine cmd("Textual Cosmic",' ',version);

		std::vector<unsigned> allowed_players = {3,4,5};
		TCLAP::ValuesConstraint<unsigned> allowed_players_constraint(allowed_players);
		TCLAP::ValueArg<unsigned> num_players_arg("n","num_players","Number of players for this game",true,3,&allowed_players_constraint);
		cmd.add(num_players_arg);

		TCLAP::SwitchArg full_control_arg("f","full_control","Force full control for all players",false);
		cmd.add(full_control_arg);

		TCLAP::ValueArg<unsigned> winning_score_arg("w","winning_score","Number of foreign colonies required to win the game",false,5,"A number from 1-8 (5 is default). Smaller numbers result in quicker games.");
		cmd.add(winning_score_arg);

		TCLAP::ValueArg<unsigned> seed_arg("s","seed","Random seed used for shuffling and other random events (typically used for debug only)",false,(unsigned) std::time(0),"an unsigned integer");
		cmd.add(seed_arg);

		TCLAP::ValueArg<unsigned> listen_port_arg("l","listen_port","Server listening port for incoming connections",false,3074,"an unsigned integer"); //3074 is a commonly used port for games
		cmd.add(listen_port_arg);
		//Other param ideas: Ashwath's simultaneous ally selection, enforcing some mix of green/yellow/red aliens, banning or forcing certain aliens or flare cards, etc.

		cmd.parse(argc,argv);

		num_players = num_players_arg.getValue();
		full_control = full_control_arg.getValue();
		winning_score = winning_score_arg.getValue();
		std::srand(seed_arg.getValue());
		listen_port = listen_port_arg.getValue();
	}
	catch (TCLAP::ArgException &e)
	{
		std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
	}


	std::cout << "Local IP address for server: " << find_local_ip_address() << "\n";

	//TODO: Add a password just for paranoia?
	CosmicServer server(listen_port);

	server.create_listening_socket();
	server.init_server_addr();
	server.bind_listening_socket();
	server.set_linger_opts_for_listening_socket();
	server.listen();
	//TODO: Let players choose their alien color (and add a debug option that disables this feature for quicker iteration)
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
	GameState game(num_players,winning_score,server);
    	game.dump();

	if(full_control)
	{
		game.set_force_full_control();
	}

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
