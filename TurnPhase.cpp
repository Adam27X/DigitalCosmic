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

		case TurnPhase::Alliance_before_selection:
			ret = "Alliance (before alliances are declared)";
		break;

		case TurnPhase::Alliance_after_selection:
			ret = "Alliance (after alliances are declared)";
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
			assert(0 && "Invalid turn phase");
		break;
	}

	return ret;
}

TurnPhase next_phase(const TurnPhase t)
{
	switch(t)
	{
		case TurnPhase::StartTurn:
			return TurnPhase::Regroup;
		break;

		case TurnPhase::Regroup:
			return TurnPhase::Destiny;
		break;

		case TurnPhase::Destiny:
			return TurnPhase::Launch;
		break;

		case TurnPhase::Launch:
			return TurnPhase::Alliance_before_selection; //Not perfect for printing because in this case we don't care about before/after selection
		break;

		case TurnPhase::Alliance_before_selection:
			return TurnPhase::Alliance_after_selection;
		break;

		case TurnPhase::Alliance_after_selection:
			return TurnPhase::Planning_before_selection;
		break;

		case TurnPhase::Planning_before_selection:
			return TurnPhase::Planning_after_selection;
		break;

		case TurnPhase::Planning_after_selection:
			return TurnPhase::Reveal;
		break;

		case TurnPhase::Reveal:
			return TurnPhase::Resolution;
		break;

		case TurnPhase::Resolution:
			return TurnPhase::StartTurn;
		break;

		default:
			assert(0 && "Invalid turn phase");
		break;
	}
}

