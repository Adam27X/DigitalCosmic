#pragma once

#include <vector>
#include <memory>

#include "CosmicDeck.hpp"
#include "PlayerColors.hpp"
#include "AlienBase.hpp"

typedef std::vector<PlayerColors> PlanetInfo;

class GameState;

class PlayerInfo
{
public:
	PlayerColors color;
	unsigned score; //TODO: Provide a function to compute the score from the planet information below?
	std::vector<PlanetInfo> planets; //Each planet has some number of ships of each valid player color
	std::unique_ptr<AlienBase> alien;
	std::vector<CosmicCardType> hand;
	EncounterRole current_role;

	void make_default_player(const PlayerColors c);
	void dump_hand() const;
	bool has_encounter_cards_in_hand() const;
	GameEvent can_respond(TurnPhase t, GameEvent g);
	GameEvent must_respond(TurnPhase t, GameEvent g);
	void set_game_state(GameState *g) { game = g; }
	CosmicCardType choose_encounter_card();

	//Does this need a forward decl?
	GameState *game; //Intended for callbacks and should be used sparingly
};

