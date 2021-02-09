#pragma once

#include <vector>
#include "AlienBase.hpp"

class Machine : public AlienBase
{
public:
	Machine();
	//Machine can only use its power on an empty stack but we model it's usage between turns automatically so return false for check_for_game_event
	bool check_for_game_event(const EncounterRole e, const TurnPhase t) const override { return false; }
	bool can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const override { return false; }
	std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player, GameEvent &this_event, const GameEvent responding_to) override;
};

