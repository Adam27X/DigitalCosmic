#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>

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

std::string to_string(const DestinyCardType &c)
{
	std::string ret;
	switch(c)
	{
		case DestinyCardType::Red:
			ret = "Red";
		break;

		case DestinyCardType::Blue:
			ret = "Blue";
		break;

		case DestinyCardType::Purple:
			ret = "Purple";
		break;

		case DestinyCardType::Yellow:
			ret = "Yellow";
		break;

		case DestinyCardType::Green:
			ret = "Green";
		break;

		case DestinyCardType::Special_fewest_ships_in_warp:
			ret = "Special: Fewest ships in warp";
		break;

		case DestinyCardType::Special_most_cards_in_hand:
			ret = "Special: Most cards in hand";
		break;

		case DestinyCardType::Special_most_foreign_colonies:
			ret = "Special: Most foreign colonies";
		break;

		case DestinyCardType::Wild:
			ret = "Wild";
		break;

		default:
			assert(0 && "Invalid Destiny card type!");
		break;
	}

	return ret;
}

DestinyDeck::DestinyDeck(unsigned nplayers)
{
	//Again we're assigning colors in a specific order here
	const unsigned destiny_cards_per_player = 3;
	deck.insert(deck.end(),destiny_cards_per_player,DestinyCardType::Red);
	deck.insert(deck.end(),destiny_cards_per_player,DestinyCardType::Blue);
	if(nplayers > 2)
		deck.insert(deck.end(),destiny_cards_per_player,DestinyCardType::Purple);
	if(nplayers > 3)
		deck.insert(deck.end(),destiny_cards_per_player,DestinyCardType::Yellow);
	if(nplayers > 4)
		deck.insert(deck.end(),destiny_cards_per_player,DestinyCardType::Green);

	//Add in cards that aren't specific to a player
	deck.push_back(DestinyCardType::Special_fewest_ships_in_warp);
	deck.push_back(DestinyCardType::Special_most_cards_in_hand);
	deck.push_back(DestinyCardType::Special_most_foreign_colonies);
	const unsigned destiny_wild_cards = 2;
	deck.insert(deck.end(),destiny_wild_cards,DestinyCardType::Wild);

	shuffle();
}

void DestinyDeck::shuffle()
{
	std::random_shuffle(deck.begin(),deck.end());
}

void DestinyDeck::dump() const
{
	std::cout << "Destiny deck:\n";
	for(auto i=deck.begin(),e=deck.end();i!=e;++i)
	{
		std::cout << to_string(*i) << "\n";
	}
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
    	std::cout << "Textual Cosmic\n";
    	GameState game(3);
    	game.dump();
	game.dump_destiny_deck();

    	return 0;
}
