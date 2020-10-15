#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>
#include <cstdlib>
#include <ctime>

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

PlayerInfo GameState::make_default_player(const PlayerColors color)
{
	PlayerInfo player;
	player.score = 0;

	player.color = color;
	const unsigned num_planets_per_player = 5;
	const unsigned default_ships_per_planet = 4;
	//PlanetInfo default_planets(num_planets_per_player,std::make_pair(player.color,default_ships_per_planet));
	player.planets.resize(num_planets_per_player);
	for(unsigned i=0; i<player.planets.size(); i++)
	{
		player.planets[i].push_back(std::make_pair(player.color,default_ships_per_planet));
	}

	return player;
}

GameState::GameState(unsigned nplayers) : num_players(nplayers), destiny_deck(DestinyDeck(nplayers))
{
	assert(nplayers > 1 && nplayers < 6 && "Invalid number of players!");
	std::cout << "Starting Game with " << num_players << " players\n";

	//For now assign colors in a specific order...TODO: let the user choose colors
	players.resize(num_players);
	players[0] = make_default_player(PlayerColors::Red);
	players[1] = make_default_player(PlayerColors::Blue);
	if(num_players > 2)
	{
		players[2] = make_default_player(PlayerColors::Purple);
	}
	if(num_players > 3)
	{
		players[3] = make_default_player(PlayerColors::Yellow);
	}
	if(num_players > 4)
	{
		players[4] = make_default_player(PlayerColors::Green);
	}
}

void GameState::dump() const
{
	std::cout << "Current scores:\n";
	for(auto i=players.begin(),e=players.end();i!=e;++i)
	{
		std::cout << to_string(i->color) << " Player score: " << i->score << "\n";
		std::cout << "Planets: {";
		for(auto ii=i->planets.begin(),ee=i->planets.end();ii!=ee;++ii)
		{
			if(ii != i->planets.begin())
				std::cout << ",";
			std::cout << "{";
			for(auto iii=ii->begin(),eee=ii->end();iii!=eee;++iii)
			{
				if(iii != ii->begin())
					std::cout << ",";
				std::cout << iii->second << "(" << to_string(iii->first) << ")";
			}
			std::cout << "}";
		}
		std::cout << "}\n";
	}
}

void GameState::dump_destiny_deck() const
{
	destiny_deck.dump();
}

void GameState::shuffle_destiny_deck()
{
	destiny_deck.shuffle();
}

int main()
{
	std::srand(unsigned (std::time(0)));
    	std::cout << "Textual Cosmic\n\n";
    	GameState game(3);
    	game.dump();
	game.dump_destiny_deck();

    	return 0;
}
