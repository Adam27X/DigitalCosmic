#pragma once

#include <vector>
#include "AlienBase.hpp"

class Human : public AlienBase
{
public:
	Human();
	std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player, const GameEvent ge) override;
	std::function<void()> get_callback_if_countered(GameState *g, const PlayerColors player) override;
};

