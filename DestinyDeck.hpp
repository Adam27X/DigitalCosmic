#pragma once

#include <vector>

#include "PlayerColors.hpp"

enum class DestinyCardType
{
	Red, //3 for each player
	Blue,
	Purple,
	Yellow,
	Green,
	Special_fewest_ships_in_warp, //1
	Special_most_cards_in_hand, //1
	Special_most_foreign_colonies, //1
	Wild //2

};

std::string to_string(const DestinyCardType &c);

PlayerColors to_PlayerColors(const DestinyCardType &c);

class DestinyDeck
{
public:
	DestinyDeck(unsigned nplayers);
	void shuffle();
	void dump() const;
	PlayerColors draw_for_first_player_and_shuffle();
	DestinyCardType draw();
	const std::string get_discard() const;
private:
	std::vector<DestinyCardType> deck;
	std::vector<DestinyCardType> discard;
};

