#pragma once

#include <string>

enum class TurnPhase
{
	StartTurn,
	Regroup,
	Destiny,
	Launch,
	Alliance,
	Planning,
	Reveal,
	Resolution
};

std::string to_string(const TurnPhase &t);

