#pragma once

#include <string>
#include "PlayerColors.hpp"

//Game actions that go on a stack and can be responded to
enum class GameEventType
{
	DrawCard,
	AlienPower
};

std::string to_string(const GameEventType &g);

class GameEvent
{
public:
	GameEvent(PlayerColors c, GameEventType g) : player(c), event_type(g) { }

	PlayerColors player;
	GameEventType event_type;
};

