#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <sstream>

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

PlayerColors to_PlayerColors(const DestinyCardType &c)
{
	switch(c)
	{
		case DestinyCardType::Red:
			return PlayerColors::Red;
		break;

		case DestinyCardType::Blue:
			return PlayerColors::Blue;
		break;

		case DestinyCardType::Purple:
			return PlayerColors::Purple;
		break;

		case DestinyCardType::Yellow:
			return PlayerColors::Yellow;
		break;

		case DestinyCardType::Green:
			return PlayerColors::Green;
		break;

		default:
			return PlayerColors::Invalid;
		break;
	}
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

PlayerColors DestinyDeck::draw_for_first_player_and_shuffle()
{
	PlayerColors ret = PlayerColors::Red; //Avoid maybe uninitialized warning
	for(auto i=deck.begin(),e=deck.end();i!=e;++i)
	{
		if(*i == DestinyCardType::Red)
		{
			ret = PlayerColors::Red;
			break;
		}
		else if(*i == DestinyCardType::Blue)
		{
			ret = PlayerColors::Blue;
			break;
		}
		else if(*i == DestinyCardType::Purple)
		{
			ret = PlayerColors::Purple;
			break;
		}
		else if(*i == DestinyCardType::Yellow)
		{
			ret = PlayerColors::Yellow;
			break;
		}
		else if(*i == DestinyCardType::Green)
		{
			ret = PlayerColors::Green;
			break;
		}
	}

	shuffle();

	return ret;
}

DestinyCardType DestinyDeck::draw()
{
	if(deck.empty())
	{
		std::copy(discard.begin(),discard.end(),deck.begin());
		discard.clear();
	}

	DestinyCardType ret = deck[0];
	discard.push_back(ret);
	deck.erase(deck.begin());

	return ret;
}

const std::string DestinyDeck::get_discard() const
{
	std::stringstream ret;
	ret << "Destiny discard pile: {";
	for(auto i=discard.begin(),e=discard.end();i!=e;++i)
	{
		if(i != discard.begin())
			ret << ",";
		ret << to_string(*i);
	}
	ret << "}\n";

	return ret.str();
}
