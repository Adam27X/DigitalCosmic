#pragma once

#include <vector>
#include <memory>

#include "CosmicDeck.hpp"
#include "PlayerColors.hpp"
#include "AlienBase.hpp"

typedef std::vector< std::pair<PlayerColors,unsigned> > PlanetInfo;

class PlayerInfo
{
public:
	PlayerColors color;
	unsigned score; //TODO: Provide a function to compute the score from the planet information below?
	std::vector<PlanetInfo> planets; //Each planet has some number of ships of each valid player color
	std::unique_ptr<AlienBase> alien;
	std::vector<CosmicCardType> hand;
	EncounterRole current_role;

	void dump_hand() const;
	bool has_encounter_cards_in_hand() const;
	GameEventType can_respond(TurnPhase t, GameEvent g);
	GameEventType must_respond(TurnPhase t, GameEvent g);
};

