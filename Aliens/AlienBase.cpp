#include <cassert>
#include <iostream>
#include "AlienBase.hpp"

void AlienBase::dump() const
{
	std::cout << "===== " << name << " =====\n";
	std::cout << "You have the power of " << power << "\n";
	std::cout << "Required player role for this power: " << to_string(role) << "\n";
	std::cout << "Is this power mandatory? " << mandatory << "\n";
	std::cout << "Valid phases for this power: {";
	for(auto i=valid_phases.begin(),e=valid_phases.end();i!=e;++i)
	{
		if(i!=valid_phases.begin())
			std::cout << ",";
		std::cout << to_string(*i);
	}
	std::cout << "}\n";
	std::cout << "Description: " << description << "\n";
}

bool AlienBase::check_encounter_role(const EncounterRole e) const
{
	if(role == PlayerRole::AnyPlayer)
		return true;

	switch(e)
	{
		case EncounterRole::Offense:
			if(role == PlayerRole::MainPlayer || role == PlayerRole::Offense || role == PlayerRole::MainPlayerOrAlly)
			{
				return true;
			}
			else if(role == PlayerRole::NotMainPlayer || role == PlayerRole::NotOffense)
			{
				return false;
			}
			else
			{
				assert(0 && "Unexpected player role for Alien when resolving check_encounter_role as offense!");
			}
		break;

		case EncounterRole::Defense:
			if(role == PlayerRole::MainPlayer || role == PlayerRole::MainPlayerOrAlly || role == PlayerRole::NotOffense)
			{
				return true;
			}
			else if(role == PlayerRole::Offense || role == PlayerRole::NotMainPlayer)
			{
				return false;
			}
			else
			{
				assert(0 && "Unexpected player role for Alien when resolving check_encounter_role as defense!");
			}
		break;

		case EncounterRole::OffensiveAlly:
		case EncounterRole::DefensiveAlly:
			if(role == PlayerRole::MainPlayerOrAlly || role == PlayerRole::NotOffense || role == PlayerRole::NotMainPlayer)
			{
				return true;
			}
			else if(role == PlayerRole::Offense || role == PlayerRole::MainPlayer)
			{
				return false;
			}
			else
			{
				assert(0 && "Unexpected player role for Alien when resolving check_encounter_role as ally!");
			}
		break;

		case EncounterRole::None:
			if(role == PlayerRole::NotOffense || role == PlayerRole::NotMainPlayer)
			{
				return true;
			}
			else if(role == PlayerRole::Offense || role == PlayerRole::MainPlayer || role == PlayerRole::MainPlayerOrAlly)
			{
				return false;
			}
			else
			{
				assert(0 && "Unexpected player role for Alien when resolving check_encounter_role as bystander!");
			}
		break;

		default:
			assert(0 && "Unexpected EncounterRole given to check_encounter_role!");
		break;
	}
}

bool AlienBase::check_for_game_event(const EncounterRole e, const TurnPhase t) const
{
	if(check_encounter_role(e) && valid_phases.find(t) != valid_phases.end())
	{
		return true;
	}

	return false;
}

bool AlienBase::can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const
{
	if(valid_phases.find(t) == valid_phases.end())
	{
		return false;
	}

	//TODO: Use check_encounter_role() here? The logic is essentially the same as check_for_game_event() above but this case is strictly for responses where the above is for an empty stack...
	switch(e)
	{
		case EncounterRole::Offense:
			if(role == PlayerRole::AnyPlayer || role == PlayerRole::MainPlayer || role == PlayerRole::Offense || role == PlayerRole::MainPlayerOrAlly)
			{
				return true;
			}
		break;

		case EncounterRole::Defense:
			if(role == PlayerRole::AnyPlayer || role == PlayerRole::MainPlayer || role == PlayerRole::MainPlayerOrAlly || role == PlayerRole::NotOffense)
			{
				return true;
			}
		break;

		case EncounterRole::OffensiveAlly:
			if(role == PlayerRole::AnyPlayer || role == PlayerRole::MainPlayerOrAlly || role == PlayerRole::NotMainPlayer || role == PlayerRole::NotOffense)
			{
				return true;
			}
		break;

		case EncounterRole::DefensiveAlly:
			if(role == PlayerRole::AnyPlayer || role == PlayerRole::MainPlayerOrAlly || role == PlayerRole::NotMainPlayer || role == PlayerRole::NotOffense)
			{
				return true;
			}
		break;

		case EncounterRole::None:
			if(role == PlayerRole::AnyPlayer)
			{
				return true;
			}
		break;

		default:
			assert(0 && "Invalid Encounter role!");
		break;
	}

	return false;
}

