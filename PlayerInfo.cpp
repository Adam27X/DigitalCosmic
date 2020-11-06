#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>
#include <sstream>

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
	alien_zapped = false;
}

void PlayerInfo::dump_hand() const
{
	std::string hand_info = get_hand();
	std::cout << hand_info;
}

std::string PlayerInfo::get_hand() const
{
	std::stringstream ret;
	ret << "Hand for the " << to_string(color) << " player:\n";
	for(auto i=hand.begin(),e=hand.end();i!=e;++i)
	{
		ret << to_string(*i) << "\n";
	}
	ret << "\n";

	return ret.str();
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

//If the player doesn't have colonies on three of their own planets they they can't respond with an Alien power
//The alien is also disabled for the remainder of an encounter in which it was zapped
bool PlayerInfo::alien_enabled() const
{
	if(alien_zapped)
	{
		return false;
	}

	unsigned num_home_colonies = 0;
	for(unsigned planet=0; planet<planets.size(); planet++)
	{
		for(unsigned ship=0; ship<planets[planet].size(); ship++)
		{
			if(planets[planet][ship] == color)
			{
				num_home_colonies++;
				break;
			}
		}
	}

	if(num_home_colonies < 3)
	{
		//TODO: Would be nice to broadcast this information
		std::cout << "The " << to_string(color) << " player cannot use their alien power because they do not control at least three of their home planets!\n";
		return false;
	}
	else
	{
		return true;
	}
}

std::vector<GameEvent> PlayerInfo::can_respond(TurnPhase t, GameEvent g)
{
	std::vector<GameEvent> vret;
	if(alien->can_respond(current_role,t,g,color) && alien_enabled())
	{
		GameEvent ret  = GameEvent(color,GameEventType::AlienPower);
		ret.callback_if_resolved = alien->get_resolution_callback(game,color,g);
		vret.push_back(ret);
	}

	if(g.event_type == GameEventType::AlienPower)
	{
		//We can respond if we have a CosmicZap
		//If the zap resolves, the zapped alien is invalid until the next encounter
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CosmicZap)
			{
				GameEvent ret = GameEvent(color,GameEventType::CosmicZap);
				ret.callback_if_resolved = [this,g] () { this->game->set_invalidate_next_callback(true); this->game->zap_alien(g.player); };
				ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
				vret.push_back(ret);
			}
		}
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
				vret.push_back(ret);
			}
		}
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
				vret.push_back(ret);
			}
		}
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
				vret.push_back(ret);
			}
		}
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
				vret.push_back(ret);
			}
		}
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
				vret.push_back(ret);
			}
		}
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
				vret.push_back(ret);
			}
		}
	}
	else if(g.event_type == GameEventType::Reinforcement2 || g.event_type == GameEventType::Reinforcement3 || g.event_type == GameEventType::Reinforcement5)
	{
		//We can respond with another reinforcement card, but only if we are a main player or ally
		if(current_role != EncounterRole::None)
		{
			for(auto i=hand.begin(),e=hand.end();i!=e;++i)
			{
				if(*i == CosmicCardType::Reinforcement2)
				{
					GameEvent ret = GameEvent(color,GameEventType::Reinforcement2);
					ret.callback_if_resolved = [this] () { this->game->add_reinforcements(this->color,2); };
					ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
					vret.push_back(ret);
				}
				else if(*i == CosmicCardType::Reinforcement3)
				{
					GameEvent ret = GameEvent(color,GameEventType::Reinforcement3);
					ret.callback_if_resolved = [this] () { this->game->add_reinforcements(this->color,3); };
					ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
					vret.push_back(ret);
				}
				else if(*i == CosmicCardType::Reinforcement5)
				{
					GameEvent ret = GameEvent(color,GameEventType::Reinforcement5);
					ret.callback_if_resolved = [this] () { this->game->add_reinforcements(this->color,5); };
					ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
					vret.push_back(ret);
				}
			}
		}
	}
	else if(g.event_type == GameEventType::Quash)
	{
		//We can respond if we have a CardZap
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CardZap)
			{
				GameEvent ret = GameEvent(color,GameEventType::CardZap);
				ret.callback_if_resolved = [this] () { this->game->set_invalidate_next_callback(true); };
				ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
				vret.push_back(ret);
			}
		}
	}
	else if(g.event_type == GameEventType::IonicGas)
	{
		//We can respond if we have a CardZap
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CardZap)
			{
				GameEvent ret = GameEvent(color,GameEventType::CardZap);
				ret.callback_if_resolved = [this] () { this->game->set_invalidate_next_callback(true); };
				ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
				vret.push_back(ret);
			}
		}
	}
	else if(g.event_type == GameEventType::SuccessfulNegotiation) //Negotiation was successful, but hasn't resolved yet (can still be quashed)
	{
		//We can respond with a Quash
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::Quash)
			{
				GameEvent ret = GameEvent(color,GameEventType::Quash);
				ret.callback_if_resolved = [this] () { this->game->get_deal_params().successful = false; };
				ret.callback_if_action_taken = [this,i] () { this->game->add_to_discard_pile(*i); this->hand.erase(i); };
				vret.push_back(ret);
			}
		}
	}
	else if(g.event_type == GameEventType::DrawCard || g.event_type == GameEventType::RetrieveWarpShip || g.event_type == GameEventType::SuccessfulDeal || g.event_type == GameEventType::DefensiveEncounterWin) //SuccessfulDeal -> Negotiation was successful and has completed (was not quashed)
	{
		//Nothing to do here, these events can only be responded to by Alien powers and that's already been taken care of
	}
	else
	{
		assert(0);
	}

	return vret;
}

GameEvent PlayerInfo::can_use_alien_with_empty_stack(const TurnPhase t)
{
	if(alien->check_for_game_event(current_role,t) && alien_enabled())
	{
		GameEvent ret  = GameEvent(color,GameEventType::AlienPower);
		GameEvent bogus = GameEvent(PlayerColors::Invalid,GameEventType::None); //Some Aliens need to know which GameEvent they're responding to in order to take the correct action (Remora). In this case, there is no event to respond to
		ret.callback_if_resolved = alien->get_resolution_callback(game,color,bogus);
		ret.callback_if_countered = alien->get_callback_if_countered(game,color);
		return ret;
	}

	return GameEvent(color,GameEventType::None);
}
