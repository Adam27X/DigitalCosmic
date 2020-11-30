#include <cassert>
#include <iostream>
#include <sstream>

#include "AlienBase.hpp"
#include "GameState.hpp"

const std::string AlienBase::get_desc() const
{
	std::stringstream desc;
	desc << "===== " << name << " =====\n";
	desc << "You have the power of " << power << "\n";
	desc << "Required player role for this power: " << to_string(role) << "\n";
	if(mandatory)
	{
		desc << "This power is mandatory.\n";
	}
	else
	{
		desc << "This power is optional.\n";
	}
	desc << "Valid phases for this power: {";
	for(auto i=valid_phases.begin(),e=valid_phases.end();i!=e;++i)
	{
		if(i!=valid_phases.begin())
			desc << ",";
		desc << to_string(*i);
	}
	desc << "}\n";
	desc << "Description: " << description << "\n\n";

	return desc.str();
}

void AlienBase::dump() const
{
	std::cout << get_desc();
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

//"Safe" function that won't be overriden
bool AlienBase::check_role_and_phase(const EncounterRole e, const TurnPhase t) const
{
	if(check_encounter_role(e) && valid_phases.find(t) != valid_phases.end())
	{
		return true;
	}

	return false;
}

//Default behavior to see if the Alien can be played with an empty stack; other functions that want to do a similar test should call check_role_and_phase() directly
bool AlienBase::check_for_game_event(const EncounterRole e, const TurnPhase t) const
{
	return check_role_and_phase(e,t);
}

std::function<void()> AlienBase::get_callback_if_action_taken(GameState *g, const PlayerColors player)
{
	if(!revealed)
	{
		std::stringstream message;
		message << "The " << to_string(player) << " player is the " << name << "!\n";
		message << name << " has the power of " << power << "\n";
		message << "Required player role for this power: " << to_string(role) << "\n";
		message << "Is this power mandatory? " << mandatory << "\n";
		message << "Valid phases for this power: {";
		for(auto i=valid_phases.begin(),e=valid_phases.end();i!=e;++i)
		{
			if(i!=valid_phases.begin())
				message << ",";
			message << to_string(*i);
		}
		message << "}\n";
		message << "Description: " << description << "\n";
		const std::string msg = message.str();

		std::function<void()> ret;
		ret = [g,player,msg,this] () { g->get_server().broadcast_message(msg); this->revealed = true; };
		return ret;
	}
	else
	{
		return nullptr;
	}
}

