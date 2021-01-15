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
			planets.planet_push_back(i,color);
		}
	}
	alien_zapped = false;
	used_flare_this_turn = false;
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
	ret << "[player_hand]\n";
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
		for(unsigned ship=0; ship<planets.planet_size(planet); ship++)
		{
			if(planets.get_ship(planet,ship) == color)
			{
				num_home_colonies++;
				break;
			}
		}
	}

	if(num_home_colonies < 3)
	{
		std::stringstream msg;
		msg << "The " << to_string(color) << " player cannot use their alien power because they do not control at least three of their home planets!\n";
		game->get_server().broadcast_message(msg.str());
		return false;
	}
	else
	{
		return true;
	}
}

bool PlayerInfo::alien_revealed() const
{
	return alien->get_revealed();
}

void PlayerInfo::discard_card_callback(const CosmicCardType c)
{
	game->add_to_discard_pile(c);

	bool card_found = false;
	for(auto i=hand.begin(),e=hand.end();i!=e;++i)
	{
		if(*i == c)
		{
			card_found = true;

			hand_erase(i);
			break;
		}
	}

	if(!card_found)
	{
		std::cerr << "Tried to erase " << to_string(c) << " from the " << to_string(color) << " player's hand, but couldn't find it!\n";
		assert(0);
	}
}

void PlayerInfo::add_card_zap_response(std::vector<GameEvent> &vret, const CosmicCardType c)
{
	GameEvent ret = GameEvent(color,GameEventType::CardZap);
	ret.callback_if_resolved = set_invalidate_next_callback_helper();
	ret.callback_if_action_taken = [this,c] { this->discard_card_callback(c); };
	vret.push_back(ret);
}

std::function<void()> PlayerInfo::set_invalidate_next_callback_helper() const
{
	return [this] () { this->game->set_invalidate_next_callback(true); };
}

void PlayerInfo::can_respond(TurnPhase t, GameEvent g, std::vector<GameEvent> &vret)
{
	if(alien->can_respond(current_role,t,g,color) && alien_enabled())
	{
		GameEvent ret  = GameEvent(color,GameEventType::AlienPower);
		ret.callback_if_resolved = alien->get_resolution_callback(game,color,ret,g);
		ret.callback_if_action_taken = alien->get_callback_if_action_taken(game,color);
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
				const CosmicCardType c = *i;
				ret.callback_if_action_taken = [this,c] { discard_card_callback(c); };
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
				add_card_zap_response(vret,*i);
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
				add_card_zap_response(vret,*i);
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
				add_card_zap_response(vret,*i);
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
				add_card_zap_response(vret,*i);
			}
			//NOTE: More generally, we can respond with any card that can be played during the same phase as the plague (regroup)
			//For now we sort of implicitly respect this rule but it would be better to use a more explicit approach that checks the phases
			else if(*i == CosmicCardType::MobiusTubes) //Respond to the Plague with Mobius Tubes (to avoid discarding it, for instance)
			{
				GameEvent ret = GameEvent(color,GameEventType::MobiusTubes);
				ret.callback_if_resolved = [this] () { this->game->free_all_ships_from_warp(); };
				const CosmicCardType c = *i;
				ret.callback_if_action_taken = [this,c] { discard_card_callback(c); };
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
				add_card_zap_response(vret,*i);
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
				add_card_zap_response(vret,*i);
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
					vret.push_back(ret);
					GameEvent &ret_ref = vret.back();
					const CosmicCardType c = *i;
					ret_ref.callback_if_action_taken = [this,&ret_ref,c] { this->game->setup_reinforcements(ret_ref); discard_card_callback(c); };
					ret_ref.callback_if_resolved = [this,&ret_ref] () { this->game->add_reinforcements(ret_ref,2); }; //This lambda needs to capture a reference to g because we want it to have access to the object altered by setup_reinforcements, which will occur later
				}
				else if(*i == CosmicCardType::Reinforcement3)
				{
					GameEvent ret = GameEvent(color,GameEventType::Reinforcement3);
					vret.push_back(ret);
					GameEvent &ret_ref = vret.back();
					const CosmicCardType c = *i;
					ret_ref.callback_if_action_taken = [this,&ret_ref,c] { this->game->setup_reinforcements(ret_ref); discard_card_callback(c); };
					ret_ref.callback_if_resolved = [this,&ret_ref] () { this->game->add_reinforcements(ret_ref,3); }; //This lambda needs to capture a reference to g because we want it to have access to the object altered by setup_reinforcements, which will occur later
				}
				else if(*i == CosmicCardType::Reinforcement5)
				{
					GameEvent ret = GameEvent(color,GameEventType::Reinforcement5);
					vret.push_back(ret);
					GameEvent &ret_ref = vret.back();
					const CosmicCardType c = *i;
					ret_ref.callback_if_action_taken = [this,&ret_ref,c] { this->game->setup_reinforcements(ret_ref); discard_card_callback(c); };
					ret_ref.callback_if_resolved = [this,&ret_ref] () { this->game->add_reinforcements(ret_ref,5); }; //This lambda needs to capture a reference to g because we want it to have access to the object altered by setup_reinforcements, which will occur later
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
				add_card_zap_response(vret,*i);
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
				add_card_zap_response(vret,*i);
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
				const CosmicCardType c = *i;
				ret.callback_if_action_taken = [this,c] { this->discard_card_callback(c); };
				vret.push_back(ret);
			}
		}
	}
	else if(g.event_type == GameEventType::CosmicDeckShuffle)
	{
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::Flare_TickTock && (alien->get_name().compare("Tick-Tock") != 0 || !alien_enabled()) && !used_flare_this_turn && !game->check_for_used_flare(*i)) //Wild part of the flare: Can be used by aliens that aren't Tick-Tock or by Tick-Tock when he is zapped or has lost his alien power
			{
				GameEvent ret = GameEvent(color,GameEventType::Flare_TickTock_Wild);
				ret.callback_if_resolved = [this] () { this->game->establish_colony_on_opponent_planet(this->color); };
				const CosmicCardType c = *i; //Flares are only discarded when zapped
				ret.callback_if_countered = [this,c] { this->discard_card_callback(c); };
				ret.callback_if_action_taken = [this,c] () { this->used_flare_this_turn = true; this->game->insert_flare_used_this_turn(c); };
				vret.push_back(ret);
			}
		}
	}
	else if(g.event_type == GameEventType::SuccessfulDeal || g.event_type == GameEventType::EncounterWin) //For now these two events can only be responded to by the Tick-Tock flare; if that changes we'll need to handle them separately
	{
		for(auto i=hand.begin(),e=hand.end();i!=e;++i)
		{
			if(*i == CosmicCardType::Flare_TickTock && alien->get_name().compare("Tick-Tock") == 0 && alien_enabled() && !used_flare_this_turn && !game->check_for_used_flare(*i)) //Super part of the flare: Can only be used by Tick-Tock when he hasn't been zapped and hasn't lost his alien power
			{
				if((g.event_type == GameEventType::SuccessfulDeal && (game->get_offense() == color || game->get_defense() == color)) || (g.event_type == GameEventType::EncounterWin && g.player == color))
				{
					GameEvent ret = GameEvent(color,GameEventType::Flare_TickTock_Super);
					ret.callback_if_resolved = [this] () { this->game->trade_ship_for_tick_tock_token(this->color); };
					const CosmicCardType c = *i; //Flares are only discarded when zapped
					ret.callback_if_countered = [this,c] { this->discard_card_callback(c); };
					ret.callback_if_action_taken = [this,c] () { this->used_flare_this_turn = true; this->game->insert_flare_used_this_turn(c); if(!this->alien->get_revealed()) { std::string msg = this->alien->get_reveal_msg(this->color); game->get_server().broadcast_message(msg); this->alien->set_revealed(); } }; //If the alien wasn't revealed yet we'll reveal it upon the super cast
					vret.push_back(ret);
				}
			}
		}
	}
	else if(g.event_type == GameEventType::DrawCard || g.event_type == GameEventType::RetrieveWarpShip || g.event_type == GameEventType::DefensiveEncounterWin) //SuccessfulDeal -> Negotiation was successful and has completed (was not quashed)
	{
		//Nothing to do here, these events can only be responded to by Alien powers and that's already been taken care of
	}
	else
	{
		assert(0);
	}
}

GameEvent PlayerInfo::can_use_alien_with_empty_stack(const TurnPhase t)
{
	if(alien->check_for_game_event(current_role,t) && alien_enabled())
	{
		GameEvent ret  = GameEvent(color,GameEventType::AlienPower);
		GameEvent bogus = GameEvent(PlayerColors::Invalid,GameEventType::None); //Some Aliens need to know which GameEvent they're responding to in order to take the correct action (Remora). In this case, there is no event to respond to
		ret.callback_if_resolved = alien->get_resolution_callback(game,color,ret,bogus);
		ret.callback_if_countered = alien->get_callback_if_countered(game,color);
		ret.callback_if_action_taken = alien->get_callback_if_action_taken(game,color);
		return ret;
	}

	return GameEvent(color,GameEventType::None);
}

const std::string PlayerInfo::get_alien_desc() const
{
	return alien->get_desc();
}

void PlayerInfo::update_client_hand() const
{
	game->send_player_hand(color);
	game->broadcast_player_hand_size(color);
}

void PlayerInfo::hand_push_back(const CosmicCardType c)
{
	hand.push_back(c);
	update_client_hand();
}

void PlayerInfo::hand_clear()
{
	hand.clear();
	update_client_hand();
}

void PlayerInfo::set_game_state(GameState *g)
{
	game = g;
	std::function<void()> callback = [this] () { this->game->update_planets(); };
	planets.set_server_callback(callback);
}

