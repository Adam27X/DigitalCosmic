#include <iostream>
#include <map>
#include <vector>
#include <cassert>

#include "TextualCosmic.hpp"

std::string to_string(const PlayerColors &p)
{
	std::string ret;
	switch(p)
	{
		case PlayerColors::Red:
			ret = "Red";
		break;

		case PlayerColors::Blue:
			ret = "Blue";
		break;

		case PlayerColors::Purple:
			ret = "Purple";
		break;

		case PlayerColors::Yellow:
			ret = "Yellow";
		break;

		case PlayerColors::Green:
			ret = "Green";
		break;

		default:
			assert(0 && "Invalid player color!");
		break;
	}

	return ret;
}

GameState::GameState(unsigned nplayers) : num_players(nplayers)
{
	assert(nplayers > 1 && nplayers < 6 && "Invalid number of players!");
	std::cout << "Starting Game with " << num_players << " players\n";
	//For now assign colors in a specific order...TODO: let the user choose colors
	const unsigned num_planets_per_player = 5;
	const unsigned default_ships_per_planet = 4;
	std::vector<unsigned> default_planets(num_planets_per_player,default_ships_per_planet);
	scores.insert(std::make_pair(PlayerColors::Red,0));
	planets.insert(std::make_pair(PlayerColors::Red,default_planets));
	scores.insert(std::make_pair(PlayerColors::Blue,0));
	planets.insert(std::make_pair(PlayerColors::Blue,default_planets));
	if(nplayers > 2)
	{
		scores.insert(std::make_pair(PlayerColors::Purple,0));
		planets.insert(std::make_pair(PlayerColors::Purple,default_planets));
	}
	if(nplayers >3)
	{
		scores.insert(std::make_pair(PlayerColors::Yellow,0));
		planets.insert(std::make_pair(PlayerColors::Purple,default_planets));
	}
	if(nplayers > 4)
	{
		scores.insert(std::make_pair(PlayerColors::Green,0));
		planets.insert(std::make_pair(PlayerColors::Purple,default_planets));
	}
}

void GameState::dump() const
{
	std::cout << "Current scores:\n";
	for(auto i=scores.begin(),e=scores.end();i!=e;++i)
	{
		std::cout << to_string(i->first) << " Player: " << i->second << "\n";
	}
	std::cout << "\nPlanet status:\n";
	for(auto i=planets.begin(),e=planets.end();i!=e;++i)
	{
		std::cout << to_string(i->first) << " Planets: {";
		for(auto ii=i->second.begin(),ee=i->second.end();ii!=ee;++ii)
		{
			if(ii != i->second.begin())
				std::cout << ",";
			std::cout << *ii;
		}
		std::cout << "}\n";
	}
}

int main()
{
    std::cout << "Textual Cosmic\n";
    GameState game(3);
    game.dump();

    return 0;
}
