#include <cassert>
#include <iostream>
#include "TurnPhase.hpp"

std::string to_string(const TurnPhase &t)
{
	std::string ret;
	switch(t)
	{
		case TurnPhase::StartTurn:
			ret = "Start Turn";
		break;

		case TurnPhase::Regroup:
			ret = "Regroup";
		break;

		case TurnPhase::Destiny:
			ret = "Destiny";
		break;

		case TurnPhase::Launch:
			ret = "Launch";
		break;

		case TurnPhase::Alliance:
			ret = "Alliance";
		break;

		case TurnPhase::Planning_before_selection:
			ret = "Planning (before cards are selected)";
		break;

		case TurnPhase::Planning_after_selection:
			ret = "Planning (after cards are selected)";
		break;

		case TurnPhase::Reveal:
			ret = "Reveal";
		break;

		case TurnPhase::Resolution:
			ret = "Resolution";
		break;

		default:
			assert(0 && "Invalid Player role for Alien!");
		break;
	}

	return ret;
}

