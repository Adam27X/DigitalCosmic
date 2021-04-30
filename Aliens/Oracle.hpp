#pragma once

#include "AlienBase.hpp"

class Oracle : public AlienBase
{
public:
	Oracle();
	//Oracle can only use its power on an empty stack
	bool can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const override { return false; }
	std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player, GameEvent &this_event, const GameEvent responding_to) override;
};

