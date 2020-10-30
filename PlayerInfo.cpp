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

//FIXME: If the player doesn't have colonies on three of their own planets they they can't respond with an Alien power
std::vector<GameEvent> PlayerInfo::can_respond(TurnPhase t, GameEvent g)
{
	std::vector<GameEvent> vret;
	if(g.event_type == GameEventType::DrawCard)
	{
		if(alien->can_respond(current_role,t,g,color))
		{
			GameEvent ret  = GameEvent(color,GameEventType::AlienPower);
			ret.callback_if_resolved = alien->get_resolution_callback(game,color,g);
			vret.push_back(ret);
		}
	}
	else if(g.event_type == GameEventType::AlienPower)
	{
		//We can respond if we have a CosmicZap
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::CosmicZap)
			{
				GameEvent ret = GameEvent(color,GameEventType::CosmicZap);
				ret.callback_if_resolved = [this] () { this->game->set_invalidate_next_callback(true); };
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
	else if(g.event_type == GameEventType::RetrieveWarpShip)
	{
		if(alien->can_respond(current_role,t,g,color))
		{
			GameEvent ret = GameEvent(color,GameEventType::AlienPower);
			ret.callback_if_resolved = alien->get_resolution_callback(game,color,g);
			vret.push_back(ret);
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
	else if(g.event_type == GameEventType::SuccessfulDeal) //Negotiation was successful and has completed (was not quashed)
	{
		if(alien->can_respond(current_role,t,g,color))
		{
			GameEvent ret = GameEvent(color,GameEventType::AlienPower);
			ret.callback_if_resolved = alien->get_resolution_callback(game,color,g);
			vret.push_back(ret);
		}
	}
	else if(g.event_type == GameEventType::DefensiveEncounterWin)
	{
		if(alien->can_respond(current_role,t,g,color))
		{
			GameEvent ret = GameEvent(color,GameEventType::AlienPower);
			ret.callback_if_resolved = alien->get_resolution_callback(game,color,g);
			vret.push_back(ret);
		}
	}
	else
	{
		assert(0);
	}

	return vret;
}

//FIXME: This function may not even be necessary
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
	else if(g.event_type == GameEventType::Reinforcement2 || g.event_type == GameEventType::Reinforcement3 || g.event_type == GameEventType::Reinforcement5)
	{
		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::Quash)
	{
		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::IonicGas)
	{
		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::SuccessfulNegotiation)
	{
		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::SuccessfulDeal)
	{
		return GameEvent(color,GameEventType::None);
	}
	else if(g.event_type == GameEventType::DefensiveEncounterWin)
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
		GameEvent bogus = GameEvent(PlayerColors::Invalid,GameEventType::None); //Some Aliens need to know which GameEvent they're responding to in order to take the correct action (Remora). In this case, there is no event to respond to
		ret.callback_if_resolved = alien->get_resolution_callback(game,color,bogus);
		ret.callback_if_countered = alien->get_callback_if_countered(game,color);
		return ret;
	}

	return GameEvent(color,GameEventType::None);
}
