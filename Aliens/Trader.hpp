#pragma once

#include <vector>
#include "AlienBase.hpp"

class Trader : public AlienBase
{
public:
	Trader();
	std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player, const GameEvent ge) override;
};

