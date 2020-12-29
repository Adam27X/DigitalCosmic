#pragma once

#include <vector>
#include "AlienBase.hpp"

class Human : public AlienBase
{
public:
	Human();
	//Human can only use its power on an empty stack
	bool can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const override { return false; }
	std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player, GameEvent &this_event, const GameEvent responding_to) override;
	std::function<void()> get_callback_if_countered(GameState *g, const PlayerColors player) override;
};

