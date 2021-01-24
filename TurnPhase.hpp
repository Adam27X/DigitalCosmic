#pragma once

#include <string>

enum class TurnPhase
{
	StartTurn,
	Regroup,
	Destiny,
	Launch,
	Alliance_before_selection,
	Alliance_after_selection,
	Planning_before_selection,
	Planning_after_selection,
	Reveal,
	Resolution
};

std::string to_string(const TurnPhase &t);

