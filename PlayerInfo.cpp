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
		for(unsigned ii=0; ii<default_ships_per_planet; ii++)
		{
			planets[i].push_back(color);
		}
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

CosmicCardType PlayerInfo::choose_encounter_card()
{
	std::cout << "Which encounter card would you like to play?\n";
	unsigned chosen_option;
	unsigned num_encounter_cards_in_hand = 0;
	for(auto i=hand.begin(),e=hand.end();i!=e;++i)
	{
		if(static_cast<unsigned>(*i) <= static_cast<unsigned>(CosmicCardType::Morph))
		{
			std::cout << num_encounter_cards_in_hand << ": " << to_string(*i) << "\n";
			num_encounter_cards_in_hand++;
		}
	}

	do
	{
		std::cout << "Please choose one of the options above.\n";
		std::cout << to_string(color) << ">>";
		std::cin >> chosen_option;
	} while(chosen_option >= num_encounter_cards_in_hand);

	unsigned option = 0;
	for(auto i=hand.begin(),e=hand.end();i!=e;++i)
	{
		if(static_cast<unsigned>(*i) <= static_cast<unsigned>(CosmicCardType::Morph))
		{
			if(option == chosen_option)
			{
				CosmicCardType ret = *i;
				hand.erase(i);
				return ret;
			}
			option++;
		}
	}

	assert(0 && "Should never get here");
}

//FIXME: Test if the Alien can respond generally (by pushing GameEventType checking into the Alien itself)
//TODO: In general multiple responses are possible...return a vector here
GameEvent PlayerInfo::can_respond(TurnPhase t, GameEvent g)
{
	if(g.event_type == GameEventType::DrawCard)
	{
		if(alien->can_respond(current_role,t,g,color))
		{
			GameEvent ret  = GameEvent(color,GameEventType::AlienPower);
			ret.callback_if_resolved = alien->get_resolution_callback(game,color);
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
				ret.callback_if_resolved = [this] () { this->game->set_invalidate_next_callback(true); };
				ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
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
			{
				GameEvent ret = GameEvent(color,GameEventType::CardZap);
				ret.callback_if_resolved = [this] () { this->game->set_invalidate_next_callback(true); };
				ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
				return ret;
			}
		}

		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::CardZap)
	{
		//Counter wars!
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CardZap)
			{
				GameEvent ret = GameEvent(color,GameEventType::CardZap);
				ret.callback_if_resolved = [this] () { this->game->set_invalidate_next_callback(true); };
				ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
				return ret;
			}
		}

		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::RetrieveWarpShip)
	{
		if(alien->can_respond(current_role,t,g,color))
		{
			GameEvent ret = GameEvent(color,GameEventType::AlienPower);
			//TODO: This functionality is specific to Remora and thus should be deduced there, if possible
			ret.callback_if_resolved = [this] () { this->game->move_ship_to_colony(this->game->get_player(this->color),this->game->get_warp()); };
			return ret;
		}
		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::MobiusTubes)
	{
		//We can respond if we have a CardZap
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CardZap)
			{
				GameEvent ret = GameEvent(color,GameEventType::CardZap);
				ret.callback_if_resolved = [this] () { this->game->set_invalidate_next_callback(true); };
				ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
				return ret;
			}
		}

		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::Plague)
	{
		//We can respond if we have a CardZap
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CardZap)
			{
				GameEvent ret = GameEvent(color,GameEventType::CardZap);
				ret.callback_if_resolved = [this] () { this->game->set_invalidate_next_callback(true); };
				ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
				return ret;
			}
		}

		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::ForceField)
	{
		//We can respond if we have a CardZap
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CardZap)
			{
				GameEvent ret = GameEvent(color,GameEventType::CardZap);
				ret.callback_if_resolved = [this] () { this->game->set_invalidate_next_callback(true); };
				ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
				return ret;
			}
		}

		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::EmotionControl)
	{
		//We can respond if we have a CardZap
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CardZap)
			{
				GameEvent ret = GameEvent(color,GameEventType::CardZap);
				ret.callback_if_resolved = [this] () { this->game->set_invalidate_next_callback(true); };
				ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
				return ret;
			}
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
	else if(g.event_type == GameEventType::RetrieveWarpShip)
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
	else if(g.event_type == GameEventType::MobiusTubes)
	{
		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::Plague)
	{
		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::ForceField)
	{
		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::EmotionControl)
	{
		return GameEvent(color,GameEventType::None);
	}
	else
	{
		assert(0);
	}
}

GameEvent PlayerInfo::can_use_alien_with_empty_stack(const TurnPhase t)
{
	if(alien->check_for_game_event(current_role,t))
	{
		GameEvent ret  = GameEvent(color,GameEventType::AlienPower);
		ret.callback_if_resolved = alien->get_resolution_callback(game,color);
		return ret;
	}

	return GameEvent(color,GameEventType::None);
}
