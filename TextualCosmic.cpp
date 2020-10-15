#include <iostream>
#include <cstdlib>
#include <ctime>

#include "TextualCosmic.hpp"

int main()
{
	std::srand(unsigned (std::time(0)));
    	std::cout << "Textual Cosmic\n\n";
    	GameState game(5);
    	game.dump();
	game.dump_destiny_deck();

    	return 0;
}
