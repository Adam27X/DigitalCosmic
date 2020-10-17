#pragma once

#include "PlayerColors.hpp"

enum class GameEventType
{
	DrawCard,
	AlienPower
};

class GameEvent
{
public:
	GameEvent(PlayerColors c, GameEventType g) : player(c), event_type(g) { }

	PlayerColors player;
	GameEventType event_type;
};

