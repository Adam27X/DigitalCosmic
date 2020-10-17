#include <iostream>
#include <cstdlib>
#include <ctime>
#include <memory>

#include "TextualCosmic.hpp"
#include "AlienBase.hpp"
#include "Aliens.hpp"

int main()
{
	//std::srand(unsigned (std::time(0))); //TODO: Used a fixed seed until this is stable
	std::srand(0);
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
	PlayerColors first_player = game.choose_first_player();
	std::cout << "The " << to_string(first_player) << " player will go first.\n";

	game.execute_turn(first_player);

	game.dump_player_hands();

    	return 0;
}
