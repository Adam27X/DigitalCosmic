#pragma once

#include <vector>
#include "AlienBase.hpp"

class Sorcerer : public AlienBase
{
public:
	Sorcerer();
	bool check_for_game_event(const EncounterRole e, const TurnPhase t) const override;
	std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player) override;
};

