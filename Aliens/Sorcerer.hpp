#pragma once

#include <vector>
#include "AlienBase.hpp"

class Sorcerer : public AlienBase
{
public:
	Sorcerer();
	std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player, const GameEvent ge) override;
};

