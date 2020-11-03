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
	if(argc == 2)
	{
		std::srand(std::stoi(argv[1]));
	}
	else
	{
		std::srand(unsigned (std::time(0)));
	}

	std::cout << "Local IP address: " << find_local_ip_address() << "\n";

	int listen_port = 1234;
	CosmicServer server(listen_port);

	server.create_listening_socket();
	server.init_server_addr();
	server.bind_listening_socket();
	server.set_linger_opts_for_listening_socket();
	server.listen();
	server.accept_client();
	server.close_listening_socket();

	//TODO: Come up with a message protocol that tells the client whether or not they need to respond
	//	Could do something like: [info] The Blue player is the offense
	//	using the '[info]' tag to designate that a response is not needed or
	//	[needs_response] Please choose one of the above options
	//	using the '[needs_response]' tag to desginate that a response is necessary
	server.send_message_to_client("Welcome!");
	std::string response;
	do
	{
		std::string message("[needs_response] Which player color would you like?\n");
		server.send_message_to_client(message);
		response = server.receive_message_from_client();

	} while(response.compare("Purple") != 0);

	//A special message that tells the client that they no longer need to listen
	std::string message("END");
	server.send_message_to_client(message);

	server.close_client();

    	std::cout << "Textual Cosmic\n\n";
    	GameState game(5);
    	game.dump();

	std::unique_ptr<AlienBase> alien(new TickTock());
	game.assign_alien(PlayerColors::Red, alien);
	std::unique_ptr<AlienBase> alien2(new Human());
	game.assign_alien(PlayerColors::Blue, alien2);
	std::unique_ptr<AlienBase> alien3(new Remora());
	game.assign_alien(PlayerColors::Purple, alien3);
	std::unique_ptr<AlienBase> alien4(new Trader());
	game.assign_alien(PlayerColors::Yellow, alien4);
	std::unique_ptr<AlienBase> alien5(new Sorcerer());
	game.assign_alien(PlayerColors::Green, alien5);

	game.deal_starting_hands();
	game.dump_player_hands();
	game.choose_first_player();

	game.start_game();

    	return 0;
}
