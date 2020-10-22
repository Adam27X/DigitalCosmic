#include <iostream>
#include <cstdlib>
#include <ctime>
#include <memory>

#include "TextualCosmic.hpp"
#include "AlienBase.hpp"
#include "Aliens.hpp"

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

	game.execute_turn();

	game.dump();

    	return 0;
}
