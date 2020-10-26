#pragma once

#include <vector>
#include "AlienBase.hpp"

class Human : public AlienBase
{
public:
	Human();
	bool check_for_game_event(const EncounterRole e, const TurnPhase t) const override;
	std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player) override;
	std::function<void()> get_callback_if_countered(GameState *g, const PlayerColors player) override;
};

