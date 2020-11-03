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

    	std::cout << "Textual Cosmic\n\n";
	GameState game(5,server);
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

	server.close_client();

    	return 0;
}
