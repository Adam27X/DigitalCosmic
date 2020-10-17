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
	return alien->can_respond(current_role,t,g,color);
}
