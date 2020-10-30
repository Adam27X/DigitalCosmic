#pragma once

#include <vector>
#include "AlienBase.hpp"

class TickTock : public AlienBase
{
public:
	TickTock();
	void discard_token();
	bool can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const override;
	std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player, const GameEvent ge) override;
private:
	unsigned num_tokens;
};

