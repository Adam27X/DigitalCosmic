#pragma once

#include <vector>
#include "AlienBase.hpp"

class Remora : public AlienBase
{
public:
	Remora();
	bool can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const override;
	bool must_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const override;
	std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player, const GameEvent ge) override;
};

