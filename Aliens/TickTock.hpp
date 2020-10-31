#pragma once

#include <vector>
#include "AlienBase.hpp"

class TickTock : public AlienBase
{
public:
	TickTock();
	void discard_token();
	//Tick-Tock cannot use its power with an empty stack
	bool check_for_game_event(const EncounterRole e, const TurnPhase t) const override { return false; }
	bool can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const override;
	std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player, const GameEvent ge) override;
private:
	unsigned num_tokens;
};

