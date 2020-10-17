#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>

#include "PlayerInfo.hpp"

void PlayerInfo::dump_hand() const
{
	std::cout << "Hand for the " << to_string(color) << " player:\n";
	for(auto i=hand.begin(),e=hand.end();i!=e;++i)
	{
		std::cout << to_string(*i) << "\n";
	}
	std::cout << "\n";
}

bool PlayerInfo::has_encounter_cards_in_hand() const
{
	for(auto i=hand.begin(),e=hand.end();i!=e;++i)
	{
		if(static_cast<unsigned>(*i) <= static_cast<unsigned>(CosmicCardType::Morph))
		{
			return true;
		}
	}

	return false;
}

bool PlayerInfo::can_respond(TurnPhase t, GameEvent g)
{
	if(g.event_type == GameEventType::DrawCard)
	{
		return alien->can_respond(current_role,t,g,color);
	}
	else if(g.event_type == GameEventType::AlienPower)
	{
		//We can respond if we have a CosmicZap
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CosmicZap)
				return true;
		}

		return false;
	}
	else
	{
		assert(0);
	}
}

bool PlayerInfo::must_respond(TurnPhase t, GameEvent g)
{
	if(g.event_type == GameEventType::DrawCard)
	{
		return alien->must_respond(current_role,t,g,color);
	}
	else if(g.event_type == GameEventType::AlienPower)
	{
		return false;
	}
	else
	{
		assert(0);
	}
}
