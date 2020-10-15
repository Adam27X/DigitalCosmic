#pragma once

#include <vector>

#include "DestinyDeck.hpp"

enum class PlayerColors
{
	Red,
	Blue,
	Purple,
	Yellow,
	Green
};

std::string to_string(const PlayerColors &p);

typedef std::vector< std::pair<PlayerColors,unsigned> > PlanetInfo;

struct PlayerInfo
{
	PlayerColors color;
	unsigned score; //TODO: Provide a function to compute the score from the planet information below?
	std::vector<PlanetInfo> planets; //Each planet has some number of ships of each valid player color
};

class GameState
{
public:
	GameState(unsigned nplayers);
	void dump() const;
	PlayerInfo make_default_player(const PlayerColors color);
	void dump_destiny_deck() const;
private:	
	void shuffle_destiny_deck();
	unsigned num_players;
	std::vector<PlayerInfo> players;
	DestinyDeck destiny_deck;
};

