#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>

#include "DestinyDeck.hpp"

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

