#pragma once

#include <vector>
#include "AlienBase.hpp"

class Remora : public AlienBase
{
public:
	Remora();
	//Remora's power cannot be used with an empty stack
	bool check_for_game_event(const EncounterRole e, const TurnPhase t) const override { return false; }
	bool can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const override;
	std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player, const GameEvent ge) override;
};

