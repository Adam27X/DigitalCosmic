#include <iostream>
#include <cstdlib>
#include <ctime>
#include <memory>

#include "TextualCosmic.hpp"
#include "AlienBase.hpp"
#include "TickTock.hpp"

int main()
{
	std::srand(unsigned (std::time(0)));
    	std::cout << "Textual Cosmic\n\n";
    	GameState game(5);
    	game.dump();
	game.dump_destiny_deck();

	std::unique_ptr<AlienBase> alien(new TickTock());
	alien->dump();

    	return 0;
}
