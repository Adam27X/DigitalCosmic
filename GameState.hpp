#pragma once

#include <vector>
#include <memory>

#include "DestinyDeck.hpp"
#include "CosmicDeck.hpp"
#include "AlienBase.hpp"

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

class PlayerInfo
{
public:
	PlayerColors color;
	unsigned score; //TODO: Provide a function to compute the score from the planet information below?
	std::vector<PlanetInfo> planets; //Each planet has some number of ships of each valid player color
	std::unique_ptr<AlienBase> alien;
	std::vector<CosmicCardType> hand;

	void dump_hand() const;
};

class GameState
{
public:
	GameState(unsigned nplayers);
	void dump() const;
	PlayerInfo make_default_player(const PlayerColors color);
	void dump_destiny_deck() const;
	void dump_cosmic_deck() const;
	void assign_alien(const PlayerColors color, std::unique_ptr<AlienBase> &alien);
	void deal_starting_hands();
	void dump_player_hands() const;
	PlayerColors choose_first_player();
private:	
	void shuffle_destiny_deck();
	void shuffle_cosmic_deck();
	unsigned num_players;
	std::vector<PlayerInfo> players;
	DestinyDeck destiny_deck;
	CosmicDeck cosmic_deck;
};

