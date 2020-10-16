#include <cassert>
#include <iostream>
#include "AlienBase.hpp"

std::string to_string(const PlayerRole &p)
{
	std::string ret;
	switch(p)
	{
		case PlayerRole::AnyPlayer:
			ret = "As any player";
		break;

		case PlayerRole::MainPlayer:
			ret = "Main player only";
		break;

		case PlayerRole::Offense:
			ret = "Offense only";
		break;

		case PlayerRole::MainPlayerOrAlly:
			ret = "Main player or ally only";
		break;

		case PlayerRole::NotMainPlayer:
			ret = "Not main player";
		break;

		case PlayerRole::NotOffense:
			ret = "Not offense";
		break;

		default:
			assert(0 && "Invalid Player role for Alien!");
		break;
	}

	return ret;
}

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

		case TurnPhase::Planning:
			ret = "Planning";
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

