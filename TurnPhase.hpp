#pragma once

#include <string>

enum class TurnPhase
{
	StartTurn,
	Regroup,
	Destiny,
	Launch,
	Alliance,
	Planning_before_selection,
	Planning_after_selection,
	Reveal,
	Resolution
};

std::string to_string(const TurnPhase &t);

