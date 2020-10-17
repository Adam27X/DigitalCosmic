#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>

#include "PlayerInfo.hpp"
#include "GameState.hpp"

void PlayerInfo::make_default_player(const PlayerColors c)
{
	score = 0;

	color = c;
	const unsigned num_planets_per_player = 5;
	const unsigned default_ships_per_planet = 4;
	planets.resize(num_planets_per_player);
	for(unsigned i=0; i<planets.size(); i++)
	{
		planets[i].push_back(std::make_pair(color,default_ships_per_planet));
	}
}

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

GameEvent PlayerInfo::can_respond(TurnPhase t, GameEvent g)
{
	if(g.event_type == GameEventType::DrawCard)
	{
		if(alien->can_respond(current_role,t,g,color))
		{
			GameEvent ret  = GameEvent(color,GameEventType::AlienPower);
			ret.callback = [this] () { this->game->draw_cosmic_card(this->game->get_player(this->color)); this->game->dump_player_hand(this->game->get_player(this->color)); };
			return ret;
		}
		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::AlienPower)
	{
		//We can respond if we have a CosmicZap
		//TODO: If Human is zapped then his side wins the encounter
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CosmicZap)
			{
				GameEvent ret = GameEvent(color,GameEventType::CosmicZap);
				ret.callback = [this] () { this->game->set_invalidate_next_callback(true); };
				return ret;
			}
		}

		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::CosmicZap)
	{
		//We can respond if we have a CardZap
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CardZap)
				return GameEvent(color,GameEventType::CardZap);
		}

		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::CardZap)
	{
		//Counter wars!
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CardZap)
				return GameEvent(color,GameEventType::CardZap);
		}

		return GameEvent(color,GameEventType::None);
	}
	else
	{
		assert(0);
	}
}

GameEvent PlayerInfo::must_respond(TurnPhase t, GameEvent g)
{
	if(g.event_type == GameEventType::DrawCard)
	{
		if(alien->must_respond(current_role,t,g,color))
		{
			return GameEvent(color,GameEventType::AlienPower);
		}
		else
		{
			return GameEvent(color,GameEventType::None);
		}
	}
	else if(g.event_type == GameEventType::AlienPower)
	{
		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::CosmicZap)
	{
		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::CardZap)
	{
		return GameEvent(color,GameEventType::None);
	}
	else
	{
		assert(0);
	}
}

