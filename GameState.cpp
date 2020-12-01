#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <limits>

#include "GameState.hpp"

bool is_only_digits(const std::string &s)
{
	        return std::all_of(s.begin(),s.end(),::isdigit);
}

GameState::GameState(unsigned nplayers, CosmicServer &serv) : num_players(nplayers), players(nplayers), destiny_deck(DestinyDeck(nplayers)), invalidate_next_callback(false), player_to_be_plagued(max_player_sentinel), is_second_encounter_for_offense(false), encounter_num(0), server(serv), warp([this] () { this->update_warp(); }), hyperspace_gate([this] () { this->update_hyperspace_gate(); }), defensive_ally_ships([this] () { this->update_defensive_ally_ships(); }), assignments([this] () { this->update_offense(); } )
{
	assert(nplayers > 1 && nplayers < max_player_sentinel && "Invalid number of players!");
	std::stringstream announce;
	announce << "Starting Game with " << num_players << " players\n";
	server.broadcast_message(announce.str());

	for(unsigned i=0; i<players.size(); i++)
	{
		players[i].set_game_state(this);
	}

	//For now assign colors in a specific order...TODO: let the user choose colors
	players[0].make_default_player(PlayerColors::Red);
	players[1].make_default_player(PlayerColors::Blue);
	if(num_players > 2)
	{
		players[2].make_default_player(PlayerColors::Purple);
	}
	if(num_players > 3)
	{
		players[3].make_default_player(PlayerColors::Yellow);
	}
	if(num_players > 4)
	{
		players[4].make_default_player(PlayerColors::Green);
	}
}

void GameState::dump() const
{
	const std::string announce = get_game_board();
	server.broadcast_message(announce);
}

const std::string GameState::get_game_board() const
{
	std::stringstream ret;
	ret << get_planets();
	ret << get_PlanetInfo(hyperspace_gate,"Hyperspace gate");
	ret << get_warp_str();
	return ret.str();
}

const std::string GameState::get_planets() const
{
	std::stringstream announce;
	announce << "Current scores:\n";
	for(auto i=players.begin(),e=players.end();i!=e;++i)
	{
		announce << to_string(i->color) << " Player score: " << i->score << "\n";
		announce << "Planets: {";
		for(auto ii=i->planets.cbegin(),ee=i->planets.cend();ii!=ee;++ii)
		{
			if(ii != i->planets.cbegin())
				announce << ",";
			announce << "{";
			for(auto iii=ii->begin(),eee=ii->end();iii!=eee;++iii)
			{
				if(iii != ii->begin())
					announce << ",";
				announce << to_string(*iii);
			}
			announce << "}";
		}
		announce << "}\n";
	}
	announce << "\n";

	return announce.str();
}

const std::string GameState::get_PlanetInfo(const PlanetInfoFull<PlayerColors> &source, const std::string name) const
{
	std::stringstream announce;
	if(source.size())
	{
		announce << name << ": {";
	}
	for(auto i=source.cbegin(),e=source.cend();i!=e;++i)
	{
		if(i!=source.cbegin())
			announce << ",";
		announce << to_string(*i);
	}
	if(source.size())
	{
		announce << "}\n\n";
	}

	return announce.str();
}

const std::string GameState::get_warp_str() const
{
	std::stringstream announce;
	if(warp.size())
	{
		announce << "Warp: {";
	}
	for(auto i=warp.cbegin(),e=warp.cend();i!=e;++i)
	{
		if(i!=warp.cbegin())
		{
			announce << ",";
		}
		announce << to_string(i->first);
	}
	if(warp.size())
	{
		announce << "}\n\n";
	}

	return announce.str();
}

void GameState::dump_planets() const
{
	const std::string announce = get_planets();
	server.broadcast_message(announce);
}

void GameState::dump_warp() const
{
	const std::string announce = get_warp_str();
	server.broadcast_message(announce);
}

void GameState::update_warp() const
{
	std::string msg("[warp_update]");
	msg.append(get_warp_str());
	server.broadcast_message(msg);
}

void GameState::update_hyperspace_gate() const
{
	std::string msg("[hyperspace_gate_update]");
	msg.append(get_PlanetInfo(hyperspace_gate, "Hyperspace gate"));
	server.broadcast_message(msg);
}

void GameState::update_defensive_ally_ships() const
{
	std::string msg("[defensive_ally_ships_update]");
	msg.append(get_PlanetInfo(defensive_ally_ships, "Defensive ally ships"));
	server.broadcast_message(msg);
}

void GameState::update_offense() const
{
	std::string msg("[offense_update] ");
	msg.append(to_string(assignments.get_offense()));
	server.broadcast_message(msg);
}

void GameState::update_planets() const
{
	std::stringstream msg;
	msg << "[planet_update]\n";
	for(auto i=players.begin(),e=players.end();i!=e;++i)
	{
		msg << to_string(i->color) << " Planets: {";
		for(auto ii=i->planets.cbegin(),ee=i->planets.cend();ii!=ee;++ii)
		{
			if(ii != i->planets.cbegin())
				msg << ",";
			msg << "{";
			for(auto iii=ii->begin(),eee=ii->end();iii!=eee;++iii)
			{
				if(iii != ii->begin())
					msg << ",";
				msg << to_string(*iii);
			}
			msg << "}";
		}
		msg << "}\n";
	}
	msg << "\n";
	server.broadcast_message(msg.str());
}

void GameState::dump_destiny_deck() const
{
	destiny_deck.dump();
}

void GameState::shuffle_destiny_deck()
{
	destiny_deck.shuffle();
}

void GameState::assign_alien(const PlayerColors color, std::unique_ptr<AlienBase> &alien)
{
	for(auto& player : players)
	{
		if(player.color == color)
		{
			player.alien = std::move(alien);
			const std::string msg = player.get_alien_desc();
			server.send_message_to_client(color,msg);
			break;
		}
	}
}

void GameState::dump_cosmic_deck() const
{
	cosmic_deck.dump();
}

void GameState::shuffle_cosmic_deck()
{
	cosmic_deck.shuffle();
}

void GameState::deal_starting_hands()
{
	const unsigned starting_hand_size = 8;
	unsigned total_cards_dealt = players.size()*starting_hand_size;
	unsigned current_cards_dealt = 0;

	auto iter = cosmic_deck.begin();
	while(total_cards_dealt > current_cards_dealt)
	{
		unsigned player_to_be_dealt_this_card = current_cards_dealt % players.size();
		players[player_to_be_dealt_this_card].hand_push_back(*iter); //Place the card in the player's hand
		iter = cosmic_deck.erase(iter); //Remove the card from the deck

		current_cards_dealt++;
	}

}

void GameState::shuffle_discard_into_cosmic_deck()
{
	assert(cosmic_deck.empty() && "Expected empty CosmicDeck when shuffling the discard pile back into the CosmicDeck");
	for(auto i=cosmic_discard.begin(),e=cosmic_discard.end();i!=e;++i)
	{
		cosmic_deck.push_back(*i);
	}
	cosmic_discard.clear();
	cosmic_deck.shuffle();
}

void GameState::draw_cosmic_card(PlayerInfo &player)
{
	//If the deck is empty, shuffle the discard deck into the deck
	if(cosmic_deck.empty())
	{
		shuffle_discard_into_cosmic_deck();
	}

	auto iter = cosmic_deck.begin();
	player.hand_push_back(*iter);
	cosmic_deck.erase(iter);

	GameEvent g(player.color,GameEventType::DrawCard);
	resolve_game_event(g);
}

void GameState::send_player_hands() const
{
	for(unsigned i=0; i<players.size(); i++)
	{
		const PlayerColors player = players[i].color;
		const std::string player_hand = get_player_const(player).get_hand();
		server.send_message_to_client(player,player_hand);
	}
}

void GameState::send_player_hand(const PlayerColors player) const
{
	const std::string player_hand = get_player_const(player).get_hand();
	server.send_message_to_client(player,player_hand);
}

void GameState::dump_player_hands() const
{
	for(auto i=players.begin(),e=players.end();i!=e;++i)
	{
		i->dump_hand();
	}
}

void GameState::dump_player_hand(const PlayerInfo &p) const
{
	p.dump_hand();
}

void GameState::choose_first_player()
{
	PlayerColors first_player = destiny_deck.draw_for_first_player_and_shuffle();
	std::stringstream announce;
	announce << "The " << to_string(first_player) << " player will go first\n";
	assignments.set_offense (first_player);
	server.broadcast_message(announce.str());
}

PlayerInfo& GameState::get_player(const PlayerColors &c)
{
	for(auto i=players.begin(),e=players.end();i!=e;++i)
	{
		if(i->color == c)
		{
			return *i;
		}
	}

	assert(0 && "Unable to find player!");
}

const PlayerInfo& GameState::get_player_const(const PlayerColors &c) const
{
	for(auto i=players.begin(),e=players.end();i!=e;++i)
	{
		if(i->color == c)
		{
			return *i;
		}
	}

	assert(0 && "Unable to find player!");
}

void GameState::discard_and_draw_new_hand(PlayerInfo &player)
{
	//Copy cards from the player's hand to discard
	for(auto i=player.hand_begin(),e=player.hand_end();i!=e;++i)
	{
		cosmic_discard.push_back(*i);
	}
	player.hand_clear();

	if(cosmic_deck.size() < 8)
	{
		//Move remaining cards in the deck to the player, then move discard to the deck, then shuffle and finish dealing the player's new hand
		for(auto i=cosmic_deck.begin(),e=cosmic_deck.end();i!=e;++i)
		{
			player.hand_push_back(*i);
		}
		cosmic_deck.clear();
		shuffle_discard_into_cosmic_deck();
	}

	while(player.hand_size() < 8)
	{
		player.hand_push_back(*cosmic_deck.begin());
		cosmic_deck.erase(cosmic_deck.begin());
	}

	//If the new hand isn't valid, try again
	if(!player.has_encounter_cards_in_hand())
	{
		return discard_and_draw_new_hand(player);
	}

	//Once we've succeeded, add the GameEvent for potential responses
	GameEvent g(player.color,GameEventType::DrawCard);
	resolve_game_event(g);
}

void GameState::free_all_ships_from_warp()
{
	while(warp.size()) //Subtle: we can use standard iteration here because the move_ship_from_warp_to_colony function calls warp.erase()
	{
		move_ship_from_warp_to_colony(get_player(warp.begin()->first));
	}
}

void GameState::cast_plague(const PlayerColors casting_player)
{
	std::vector<std::string> options;
	for(unsigned i=0; i<players.size(); i++)
	{
		options.push_back(to_string(players[i].color));
	}

	std::string prompt("Which player would you like to harm?\n");
	unsigned chosen_option = prompt_player(casting_player,prompt,options);

	assert(player_to_be_plagued == max_player_sentinel);
	player_to_be_plagued = chosen_option;
}

void GameState::cast_force_field(const PlayerColors casting_player)
{
	server.send_message_to_client(casting_player,"Which alliances would you like to end?\n");
	for(auto i=assignments.offensive_allies.begin(),e=assignments.offensive_allies.end();i!=e;++i)
	{
		std::stringstream prompt;
		prompt << to_string(i->first) << "? (allied with the offense)\n";
		std::vector<std::string> options;
		options.push_back("Y");
		options.push_back("N");
		unsigned choice = prompt_player(casting_player,prompt.str(),options);

		if(choice == 0)
		{
			allies_to_be_stopped.insert(i->first);
		}
	}

	for(auto i=assignments.defensive_allies.begin(),e=assignments.defensive_allies.end();i!=e;++i)
	{
		std::stringstream prompt;
		prompt << to_string(i->first) << "? (allied with the defense)\n";
		std::vector<std::string> options;
		options.push_back("Y");
		options.push_back("N");
		unsigned choice = prompt_player(casting_player,prompt.str(),options);

		if(choice == 0)
		{
			allies_to_be_stopped.insert(i->first);
		}
	}
}

void GameState::lose_ships_to_warp(const PlayerColors player, const unsigned num_ships)
{
	for(unsigned ship_num=0; ship_num<num_ships; ship_num++)
	{
		std::vector< std::pair<PlayerColors,unsigned> > valid_colonies = get_valid_colonies(player); //A list of planet colors and indices
		if(!valid_colonies.size())
		{
			//Check if the player has a ship on the hyperspace gate
			for(auto i=hyperspace_gate.begin(),e=hyperspace_gate.end();i!=e;++i)
			{
				if(*i == player)
				{
					warp.push_back(std::make_pair(*i,encounter_num));
					hyperspace_gate.erase(i);
					break;
				}
			}
		}
		else
		{
			std::string msg("Choose a colony to lose a ship from.\n");
			server.send_message_to_client(player,msg);
			const std::pair<PlayerColors,unsigned> chosen_colony = prompt_valid_colonies(player,valid_colonies);
			PlayerInfo &player_with_chosen_colony = get_player(chosen_colony.first);
			for(auto i=player_with_chosen_colony.planets.planet_begin(chosen_colony.second),e=player_with_chosen_colony.planets.planet_end(chosen_colony.second);i!=e;++i)
			{
				if(*i == player)
				{
					warp.push_back(std::make_pair(*i,encounter_num));
					player_with_chosen_colony.planets.planet_erase(chosen_colony.second,i);
					break;
				}
			}
		}
	}
}

void GameState::plague_player()
{
	assert(player_to_be_plagued < players.size());
	PlayerInfo &victim = players[player_to_be_plagued];

	//First they lose three ships of his or her choice to the warp (if possible)
	lose_ships_to_warp(victim.color,3);

	//Then they must discard one card of each type he or she has in hand
	unsigned chosen_option;
	//Artifacts
	std::vector<CosmicCardType> valid_artifacts;
	for(auto i=victim.hand_begin(),e=victim.hand_end();i!=e;++i)
	{
		if((*i >= CosmicCardType::CardZap) && (*i <= CosmicCardType::Quash))
		{
			valid_artifacts.push_back(*i);
		}
	}
	if(valid_artifacts.size())
	{
		std::stringstream prompt;
		prompt << "The " << to_string(victim.color) << " player has the following artifacts. Choose one to discard.\n";
		std::vector<std::string> options;
		for(unsigned i=0; i<valid_artifacts.size(); i++)
		{
			options.push_back(to_string(valid_artifacts[i]));
		}
		chosen_option = prompt_player(victim.color,prompt.str(),options);
		CosmicCardType choice = valid_artifacts[chosen_option];
		for(auto i=victim.hand_begin(),e=victim.hand_end();i!=e;++i)
		{
			if(*i == choice)
			{
				cosmic_discard.push_back(*i);
				victim.hand_erase(i);
				break;
			}
		}
	}
	//Attack
	std::vector<CosmicCardType> valid_attacks;
	for(auto i=victim.hand_begin(),e=victim.hand_end();i!=e;++i)
	{
		if((*i >= CosmicCardType::Attack0) && (*i <= CosmicCardType::Attack40))
		{
			valid_attacks.push_back(*i);
		}
	}
	if(valid_attacks.size())
	{
		std::stringstream prompt;
		prompt << "The " << to_string(victim.color) << " player has the following attack cards. Choose one to discard.\n";
		std::vector<std::string> options;
		for(unsigned i=0; i<valid_attacks.size(); i++)
		{
			options.push_back(to_string(valid_attacks[i]));
		}
		chosen_option = prompt_player(victim.color,prompt.str(),options);
		CosmicCardType choice = valid_attacks[chosen_option];
		for(auto i=victim.hand_begin(),e=victim.hand_end();i!=e;++i)
		{
			if(*i == choice)
			{
				cosmic_discard.push_back(*i);
				victim.hand_erase(i);
				break;
			}
		}
	}
	//Negotiate
	for(auto i=victim.hand_begin(),e=victim.hand_end();i!=e;++i)
	{
		if(*i == CosmicCardType::Negotiate)
		{
			cosmic_discard.push_back(*i);
			victim.hand_erase(i);
			break;
		}
	}
	//Morph
	for(auto i=victim.hand_begin(),e=victim.hand_end();i!=e;++i)
	{
		if(*i == CosmicCardType::Morph)
		{
			cosmic_discard.push_back(*i);
			victim.hand_erase(i);
			break;
		}
	}
	//Reinforcement
	std::vector<CosmicCardType> valid_reinforcements;
	for(auto i=victim.hand_begin(),e=victim.hand_end();i!=e;++i)
	{
		if((*i >= CosmicCardType::Reinforcement2) && (*i <= CosmicCardType::Reinforcement5))
		{
			valid_reinforcements.push_back(*i);
		}
	}
	if(valid_reinforcements.size())
	{
		std::stringstream prompt;
		prompt << "The " << to_string(victim.color) << " player has the following reinforcement cards. Choose one to discard.\n";
		std::vector<std::string> options;
		for(unsigned i=0; i<valid_reinforcements.size(); i++)
		{
			options.push_back(to_string(valid_reinforcements[i]));
		}
		chosen_option = prompt_player(victim.color,prompt.str(),options);
		CosmicCardType choice = valid_reinforcements[chosen_option];
		for(auto i=victim.hand_begin(),e=victim.hand_end();i!=e;++i)
		{
			if(*i == choice)
			{
				cosmic_discard.push_back(*i);
				victim.hand_erase(i);
				break;
			}
		}
	}
	//TODO: Support Flares here when they're available

	player_to_be_plagued = max_player_sentinel;
}

void GameState::stop_allies()
{
	for(auto i=allies_to_be_stopped.begin(),e=allies_to_be_stopped.end();i!=e;++i)
	{
		//Remove from assignments, update player status, and send ships back
		PlayerInfo &ally = get_player(*i);
		if(assignments.offensive_allies.find(*i) != assignments.offensive_allies.end())
		{
			unsigned num_ally_ships_sent = assignments.offensive_allies[*i];
			assignments.offensive_allies.erase(*i);
			for(unsigned num_ships=0; num_ships<num_ally_ships_sent; num_ships++)
			{
				move_ship_to_colony(ally,hyperspace_gate);
			}
		}
		else if(assignments.defensive_allies.find(*i) != assignments.defensive_allies.end())
		{
			unsigned num_ally_ships_sent = assignments.defensive_allies[*i];
			assignments.defensive_allies.erase(*i);
			for(unsigned num_ships=0; num_ships<num_ally_ships_sent; num_ships++)
			{
				move_ship_to_colony(ally,defensive_ally_ships);
			}
		}
		else
		{
			assert(0 && "Ally needs to be stopped but wasn't found in offensive or defensive allies list\n");
		}

		ally.current_role = EncounterRole::None;
	}
	allies_to_be_stopped.clear();
}

//FIXME: This function can be called multiple times per turn
//	 Consider the following scenario:
//	 Blue player is offense during regroup and choose not to cast Mobius Tubes
//	 The Green player, next to act, casts Plague on the Blue Player
//	 The Blue player has another artifact card to discard and chooses to hang on to Mobius Tubes
//	 Once plague has resolved, shouldn't the Blue player be able to play Mobius Tubes? It's still the regroup phase!
//	 More generally, if anything resolves the first time this function is called we need to call it again (what's tricky here is that we also have to be sure that we don't allow events to resolve twice, such as Alien powers...we may have to mark them as 'used')
//TODO: Prompt players even when they have no options here? Slows down the game a bit but does a better job of keeping players aware of what's going on
//	Additionally, the "None" option should probably read 'Proceed to the <next_phase> Phase' (or 'Proceed to the next encounter' for resolution)
void GameState::check_for_game_events()
{
	std::vector<PlayerColors> player_order = get_player_order();

	for(unsigned current_player_index=0; current_player_index<players.size(); current_player_index++)
	{
		PlayerInfo &current_player = get_player(player_order[current_player_index]);
		//Starting with the offense, check for valid plays (artifact cards or alien powers) based on the current TurnPhase
		std::vector<GameEvent> valid_plays;
		for(auto i=current_player.hand_begin(),e=current_player.hand_end(); i!=e; ++i)
		{
			if(can_play_card_with_empty_stack(state,*i,current_player.current_role))
			{
				GameEvent g(current_player.color,to_game_event_type(*i));
				valid_plays.push_back(g);
			}
		}

		bool alien_power_used = false;
		bool alien_power_available = false;
		bool alien_power_mandatory = false;
		GameEvent alien_power = current_player.can_use_alien_with_empty_stack(state);
		if(alien_power.event_type != GameEventType::None)
		{
			assert(alien_power.event_type == GameEventType::AlienPower);
			valid_plays.push_back(alien_power);
			alien_power_available = true;
			alien_power_mandatory = current_player.alien->get_mandatory();
		}

		//List the valid plays and ask the player if they would like to do any. Note that if they choose one they can still play another
		while(valid_plays.size())
		{
			std::stringstream prompt;
			prompt << "The " << to_string(current_player.color) << " player has the following valid plays to choose from:\n";
			std::vector<std::string> options;
			for(unsigned i=0; i<valid_plays.size(); i++)
			{
				std::stringstream opt;
				opt << to_string(valid_plays[i].event_type);
				if(valid_plays[i].event_type == GameEventType::AlienPower && alien_power_mandatory)
				{
					opt << " (mandatory)";
				}
				options.push_back(opt.str());
			}
			options.push_back("None");
			unsigned chosen_option = prompt_player(current_player.color,prompt.str(),options);

			if(chosen_option != valid_plays.size()) //An action was taken
			{
				GameEvent g = valid_plays[chosen_option];
				if(g.event_type != GameEventType::AlienPower)
				{
					CosmicCardType play = to_cosmic_card_type(g.event_type);

					//Remove this card from the player's hand and add it to discard
					add_to_discard_pile(play);
					unsigned old_hand_size = current_player.hand_size(); //Sanity checking
					for(auto i=current_player.hand_begin(),e=current_player.hand_end(); i!=e; ++i)
					{
						if(*i == play)
						{
							current_player.hand_erase(i);
							break;
						}
					}
					assert(current_player.hand_size()+1 == old_hand_size && "Error removing played card from the player's hand!");

					get_callbacks_for_cosmic_card(play,g);
				}
				else
				{
					alien_power_used = true;
				}

				if(g.callback_if_action_taken)
					g.callback_if_action_taken();
				g.callback_if_action_taken = nullptr;
				resolve_game_event(g);

				//Recalculate the set of valid plays since either the current play or a response to it could have changed it
				valid_plays.clear();
				for(auto i=current_player.hand_begin(),e=current_player.hand_end(); i!=e; ++i)
				{
					if(can_play_card_with_empty_stack(state,*i,current_player.current_role))
					{
						GameEvent g(current_player.color,to_game_event_type(*i));
						valid_plays.push_back(g);
					}
				}

				if(!alien_power_used)
				{
					GameEvent alien_power = current_player.can_use_alien_with_empty_stack(state);
					if(alien_power.event_type != GameEventType::None)
					{
						assert(alien_power.event_type == GameEventType::AlienPower);
						valid_plays.push_back(alien_power);
					}
				}
			}
			else
			{
				if(alien_power_available && alien_power_mandatory && !alien_power_used)
				{
					server.send_message_to_client(current_player.color,"Mandatory alien power not yet chosen, try again.\n");
				}
				else
				{
					break;
				}
			}
		}
	}

	//Recalculate player scores
	update_player_scores();
}

void GameState::update_player_scores()
{
	//Reset player scores
	for(auto i=players.begin(),e=players.end();i!=e;++i)
	{
		i->score = 0;
	}

	for(auto i=players.begin(),e=players.end();i!=e;++i)
	{
		for(auto ii=i->planets.begin(),ee=i->planets.end();ii!=ee;++ii)
		{
			std::set<PlayerColors> foreign_colony_on_this_planet;
			for(auto iii=ii->begin(),eee=ii->end();iii!=eee;++iii) //For each ship on this planet
			{
				if((*iii != i->color) && foreign_colony_on_this_planet.find(*iii) == foreign_colony_on_this_planet.end()) //If this ship color is a different color than the planet, it represents part of a foreign colony
				{
					PlayerInfo &player = get_player(*iii);
					player.score++;
					foreign_colony_on_this_planet.insert(*iii); //Don't count multiple ships in one foreign colony as multiple foreign colonies
				}
			}
		}
	}

	//Check to see if anyone has won the game
	bool game_over = false;
	for(auto i=players.begin(),e=players.end();i!=e;++i)
	{
		if(i->score >= 5)
		{
			game_over = true;
			std::stringstream announce;
			announce << "The " << to_string(i->color) << " player has won the game!\n";
			server.broadcast_message(announce.str());
		}
	}

	if(game_over)
	{
		dump();
		std::exit(0);
	}
}

void GameState::get_callbacks_for_cosmic_card(const CosmicCardType play, GameEvent &g)
{
	switch(play)
	{
		case CosmicCardType::MobiusTubes:
			g.callback_if_resolved = [this] () { this->free_all_ships_from_warp(); };
		break;

		case CosmicCardType::Plague:
			//Choose which player to plague
			g.callback_if_action_taken = [this,g] () { this->cast_plague(g.player); };
			//Plague them
			g.callback_if_resolved = [this] () { this->plague_player(); };
		break;

		case CosmicCardType::ForceField:
			//Choose which alliances to end
			g.callback_if_action_taken = [this,g] () { this->cast_force_field(g.player); };
			//End them
			g.callback_if_resolved = [this] () { this->stop_allies(); };
		break;

		case CosmicCardType::EmotionControl:
			g.callback_if_resolved = [this] () { this->force_negotiation(); };
		break;

		case CosmicCardType::Reinforcement2:
			g.callback_if_resolved = [this,g] () { this->add_reinforcements(g.player,2); };
		break;

		case CosmicCardType::Reinforcement3:
			g.callback_if_resolved = [this,g] () { this->add_reinforcements(g.player,3); };
		break;

		case CosmicCardType::Reinforcement5:
			g.callback_if_resolved = [this,g] () { this->add_reinforcements(g.player,5); };
		break;

		case CosmicCardType::IonicGas:
			g.callback_if_resolved = [this] () { this->assignments.stop_compensation_and_rewards = true; };
		break;

		default:
			assert(0 && "CosmicCardType callbacks not yet implemenmted\n");
		break;
	}
}

void GameState::draw_from_destiny_deck()
{
	DestinyCardType dest = destiny_deck.draw();
	const PlayerColors off = assignments.get_offense();

	if(to_PlayerColors(dest) != PlayerColors::Invalid)
	{
		//We drew a player color
		PlayerColors drawn = to_PlayerColors(dest);

		if(drawn == off)
		{
			std::stringstream announce;
			announce << "The " << to_string(off) << " player has drawn his or her own color.\n";

			PlayerInfo &offense = get_player(off);
			std::map< std::pair<PlayerColors,unsigned>, unsigned > valid_home_system_encounters; //Map <opponent,planet number> -> number of opponent ships on that planet
			for(unsigned i=0; i<offense.planets.size(); i++)
			{
				for(auto ii=offense.planets.planet_begin(i),ee=offense.planets.planet_end(i); ii!=ee; ++ii)
				{
					if(*ii != off)
					{
						if(valid_home_system_encounters.find(std::make_pair(*ii,i)) == valid_home_system_encounters.end())
						{
							valid_home_system_encounters[std::make_pair(*ii,i)] = 1;
						}
						else
						{
							valid_home_system_encounters[std::make_pair(*ii,i)]++;
						}
					}
				}
			}
			if(valid_home_system_encounters.empty())
			{
				announce << "Since there are no opponents in the offense's home system the offense must draw another destiny card\n";
				server.broadcast_message(announce.str());
				draw_from_destiny_deck();
			}
			else
			{
				server.broadcast_message(announce.str());
				std::string prompt("The offense may either have an encounter with a player in his or her own system or draw another destiny card\n");
				std::vector<std::string> options;
				unsigned option = 0;
				//TODO: It's probably better to let the offense choose between some sort of home system encounter and redrawing now and then letting them choose the specific home system encounter during launch should they go that route
				//	In practice I'm not sure that this timing will ever matter
				for(auto i=valid_home_system_encounters.begin(),e=valid_home_system_encounters.end();i!=e;++i)
				{
					std::stringstream opt;
					opt << "Have an encounter with the " << to_string(i->first.first) << " player on " << to_string(off) << " Planet " << i->first.second << " (the " << to_string(i->first.first) << " player has " << i->second << " ships on this planet.)";
					options.push_back(opt.str());
				}
				options.push_back("Draw another destiny card");
				unsigned chosen_option = prompt_player(off,prompt,options);

				if(chosen_option == valid_home_system_encounters.size())
				{
					announce.str("The offense has chosen to redraw\n");
					server.broadcast_message(announce.str());
					draw_from_destiny_deck();
				}
				else
				{
					//The offense has chosen to have an encounter against another player in their home system
					std::pair<PlayerColors,unsigned> home_system_encounter;
					option = 0;
					for(auto i=valid_home_system_encounters.begin(),e=valid_home_system_encounters.end();i!=e;++i)
					{
						if(option == chosen_option)
						{
							home_system_encounter = i->first;
						}
						option++;
					}
					assignments.defense = home_system_encounter.first;
					assignments.planet_location = off;
					assignments.planet_id = home_system_encounter.second;
					PlayerInfo &def = get_player(assignments.defense);
					def.current_role = EncounterRole::Defense;
				}
			}
		}
		else
		{
			assignments.defense = drawn;
			assignments.planet_location = drawn;
			PlayerInfo &def = get_player(assignments.defense);
			def.current_role = EncounterRole::Defense;
		}
	}
	else
	{
		//We drew some sort of special card
		if(dest == DestinyCardType::Special_fewest_ships_in_warp)
		{
			std::map<PlayerColors,unsigned> num_ships_in_warp; //Map each player to how many ships they have in the warp
			num_ships_in_warp[PlayerColors::Red] = 0;
			num_ships_in_warp[PlayerColors::Blue] = 0;
			if(num_players > 2)
				num_ships_in_warp[PlayerColors::Purple] = 0;
			if(num_players > 3)
				num_ships_in_warp[PlayerColors::Yellow] = 0;
			if(num_players > 4)
				num_ships_in_warp[PlayerColors::Green] = 0;
			for(auto i=warp.begin(),e=warp.end();i!=e;++i)
			{
				num_ships_in_warp[i->first]++;
			}
			//Tiebreak with the player closest in ascending player order
			//Start with the player closest to the offense and keep the old result during ties
			unsigned player_index = max_player_sentinel;
			for(unsigned i=0; i<players.size(); i++)
			{
				if(players[i].color == off)
				{
					player_index = i;
					break;
				}
			}
			unsigned defense_index = max_player_sentinel;
			unsigned min_ships_in_warp = std::numeric_limits<unsigned>::max();
			for(unsigned i=0; i<players.size(); i++)
			{
				unsigned current_player_index = (i+player_index) % players.size();
				PlayerColors current_player_color = players[current_player_index].color;
				if((current_player_color != off) && (num_ships_in_warp[current_player_color] < min_ships_in_warp))
				{
					min_ships_in_warp = num_ships_in_warp[current_player_color];
					defense_index = current_player_index;
				}
			}

			assignments.defense = players[defense_index].color;
			assignments.planet_location = players[defense_index].color;
			PlayerInfo &def = get_player(assignments.defense);
			def.current_role = EncounterRole::Defense;
		}
		else if(dest == DestinyCardType::Special_most_cards_in_hand)
		{
			//Tiebreak with the player closest in ascending player order
			//Start with the player closest to the offense and keep the old result during ties
			unsigned player_index = max_player_sentinel;
			for(unsigned i=0; i<players.size(); i++)
			{
				if(players[i].color == off)
				{
					player_index = i;
					break;
				}
			}
			unsigned defense_index = max_player_sentinel;
			unsigned most_cards_in_hand = 0;
			for(unsigned i=0; i<players.size(); i++)
			{
				unsigned current_player_index = (i+player_index) % players.size();
				PlayerColors current_player_color = players[current_player_index].color;
				if((current_player_color != off) && (players[current_player_index].hand_size() > most_cards_in_hand))
				{
					most_cards_in_hand = players[current_player_index].hand_size();
					defense_index = current_player_index;
				}
			}

			if(most_cards_in_hand == 0)
			{
				//4 way tie!
				defense_index = (player_index+1) % players.size();
			}

			assignments.defense = players[defense_index].color;
			assignments.planet_location = players[defense_index].color;
			PlayerInfo &def = get_player(assignments.defense);
			def.current_role = EncounterRole::Defense;
		}
		else if(dest == DestinyCardType::Special_most_foreign_colonies)
		{
			//Tiebreak with the player closest in ascending player order
			//Start with the player closest to the offense and keep the old result during ties
			unsigned player_index = max_player_sentinel;
			for(unsigned i=0; i<players.size(); i++)
			{
				if(players[i].color == off)
				{
					player_index = i;
					break;
				}
			}
			unsigned defense_index = max_player_sentinel;
			unsigned most_foreign_colonies = 0;
			for(unsigned i=0; i<players.size(); i++)
			{
				unsigned current_player_index = (i+player_index) % players.size();
				PlayerColors current_player_color = players[current_player_index].color;
				if((current_player_color != off) && (players[current_player_index].score > most_foreign_colonies))
				{
					most_foreign_colonies = players[current_player_index].score;
					defense_index = current_player_index;
				}
			}

			if(most_foreign_colonies == 0)
			{
				//4 way tie!
				defense_index = (player_index+1) % players.size();
			}

			assignments.defense = players[defense_index].color;
			assignments.planet_location = players[defense_index].color;
			PlayerInfo &def = get_player(assignments.defense);
			def.current_role = EncounterRole::Defense;
		}
		else if(dest == DestinyCardType::Wild)
		{
			std::string prompt("The offense has drawn a wild destiny card and may have an encounter with the player of his or her choice in their home system.\n");
			std::vector<std::string> options;
			for(unsigned i=0; i<players.size(); i++)
			{
				if(players[i].color != off)
				{
					options.push_back(to_string(players[i].color));
				}
			}
			unsigned chosen_option = prompt_player(off,prompt,options);

			unsigned valid_option = 0;
			for(unsigned i=0; i<players.size(); i++)
			{
				if(players[i].color != off)
				{
					if(chosen_option == valid_option)
					{
						assignments.defense = players[i].color;
						assignments.planet_location = players[i].color;
						break;
					}
					valid_option++;
				}
			}

			assert(assignments.defense != PlayerColors::Invalid && "Failed to set defensive player after drawing wild destiny card");
			assert(assignments.planet_location != PlayerColors::Invalid && "Failed to set defensive planet location after drawing wild destiny card");

			PlayerInfo &def = get_player(assignments.defense);
			def.current_role = EncounterRole::Defense;
		}
		else
		{
			assert(0 && "Unexpected destiny card type!");
		}
	}
}

void GameState::choose_opponent_planet()
{
	//If the offense is having an encounter on their home system they've already chosen a planet, so there's nothing to do here
	if(assignments.planet_location == assignments.get_offense())
		return;

	std::stringstream prompt;
	prompt << "The offense (" << to_string(assignments.get_offense()) << ") has chosen to have an encounter with the " << to_string(assignments.defense) << " player.\n";
	std::vector<std::string> options;
	const PlayerInfo &host = get_player(assignments.planet_location);
	for(unsigned i=0; i<host.planets.size(); i++)
	{
		std::stringstream opt;
		opt << to_string(host.color) << " Planet " << i;
		options.push_back(opt.str());
	}
	unsigned chosen_option = prompt_player(assignments.get_offense(),prompt.str(),options);
	assignments.planet_id = chosen_option;

	std::stringstream announce;
	announce << "The offense (" << to_string(assignments.get_offense()) << ") will have an encounter with the " << to_string(assignments.defense) << " player on " << to_string(assignments.planet_location) << " Planet " << assignments.planet_id << "\n";
	//TODO: Show the status of the chosen planet location?
	server.broadcast_message(announce.str());
}

void GameState::send_in_ships(const PlayerColors player)
{
	//List the player's valid colonies and have them choose a colony or none until they've chosen none or they've chosen four
	std::stringstream prompt;
	prompt << "The " << to_string(player) << " player can choose up to four ships from any of their valid colonies\n";
	unsigned launched_ships = 0;
	unsigned choice;
	std::vector< std::pair<PlayerColors,unsigned> > valid_colonies;
	do
	{
		valid_colonies = get_valid_colonies(player);
		std::vector<std::string> options;
		for(unsigned i=0; i<valid_colonies.size(); i++)
		{
			std::stringstream opt;
			opt << to_string(valid_colonies[i].first) << " planet " << valid_colonies[i].second;
			options.push_back(opt.str());
		}
		options.push_back("None");

		choice = prompt_player(player,prompt.str(),options);
		if(choice < valid_colonies.size())
		{
			if(assignments.defensive_allies.find(player) != assignments.defensive_allies.end())
			{
				defensive_ally_ships.push_back(player);
				assignments.defensive_allies[player]++; //Keep track of the number of ships sent in by each ally
			}
			else //Offense or offensive ally
			{
				if(assignments.offensive_allies.find(player) != assignments.offensive_allies.end())
				{
					assignments.offensive_allies[player]++; //Keep track of the number of ships sent in by each ally
				}
				hyperspace_gate.push_back(player); //Add the ship to the hyperspace gate
			}
			//Remove the ship from the chosen colony
			PlayerInfo &host = get_player(valid_colonies[choice].first);
			const unsigned planet_id = valid_colonies[choice].second;
			for(auto i=host.planets.planet_begin(planet_id),e=host.planets.planet_end(planet_id);i!=e;++i)
			{
				if(*i == player)
				{
					host.planets.planet_erase(planet_id,i);
					break;
				}
			}
			launched_ships++;
		}
	} while(((choice < valid_colonies.size()) && launched_ships < 4) || choice > valid_colonies.size());

	if(launched_ships == 0 && !valid_colonies.empty())
	{
		server.send_message_to_client(player,"The offense and allies *must* commit at least one ship to the encounter if able. Try again.\n");
		send_in_ships(player);
	}
}

std::set<PlayerColors> GameState::invite_allies(const std::set<PlayerColors> &potential_allies, bool offense)
{
	std::set<PlayerColors> invited;
	std::string inviter = offense ? "offense" : "defense";
	for(auto i=potential_allies.begin(),e=potential_allies.end();i!=e;++i)
	{
		std::stringstream prompt;
		prompt << "Would the " << inviter << " like to invite the " << to_string(*i) << " player as an ally?\n";
		std::vector<std::string> options;
		options.push_back("Y");
		options.push_back("N");
		PlayerColors inviter_color = offense ? assignments.get_offense() : assignments.defense;
		unsigned response = prompt_player(inviter_color,prompt.str(),options);

		if(response == 0)
		{
			invited.insert(*i);
		}
	}

	return invited;
}

void GameState::form_alliances(std::set<PlayerColors> &invited_by_offense, std::set<PlayerColors> &invited_by_defense)
{
	unsigned player_index = max_player_sentinel;
	for(unsigned i=0; i<players.size(); i++)
	{
		if(players[i].color == assignments.get_offense())
		{
			player_index = i;
			break;
		}
	}
	for(unsigned i=0; i<players.size(); i++)
	{
		unsigned current_player_index = (i+player_index) % players.size();
		PlayerColors current_player_color = players[current_player_index].color;

		if(get_valid_colonies(current_player_color).empty())
		{
			//The potential ally has no ships to commit to the encounter and therefore cannot be an ally
			continue;
		}

		if(invited_by_offense.find(current_player_color) != invited_by_offense.end())
		{
			std::stringstream prompt;
			prompt << "Would the " << to_string(current_player_color) << " like to join with the offense?\n";
			std::vector<std::string> options;
			options.push_back("Y");
			options.push_back("N");
			unsigned response = prompt_player(current_player_color,prompt.str(),options);

			if(response == 0)
			{
				assignments.offensive_allies.insert(std::make_pair(current_player_color,0));
				PlayerInfo &ally = get_player(current_player_color);
				ally.current_role = EncounterRole::OffensiveAlly;
				send_in_ships(current_player_color);
				if(invited_by_defense.find(current_player_color) != invited_by_defense.end()) //Cannot join both offense and defense as an ally, so remove from the defensive list
				{
					invited_by_defense.erase(current_player_color);
				}
			}
		}

		if(invited_by_defense.find(current_player_color) != invited_by_defense.end())
		{
			std::stringstream prompt;
			prompt << "Would the " << to_string(current_player_color) << " like to join with the defense?\n";
			std::vector<std::string> options;
			options.push_back("Y");
			options.push_back("N");
			unsigned response = prompt_player(current_player_color,prompt.str(),options);

			if(response == 0)
			{
				assignments.defensive_allies.insert(std::make_pair(current_player_color,0));
				PlayerInfo &ally = get_player(current_player_color);
				ally.current_role = EncounterRole::DefensiveAlly;
				send_in_ships(current_player_color);
				if(invited_by_offense.find(current_player_color) != invited_by_offense.end()) //Cannot join both offense and defense as an ally, so remove from the offensive list
				{
					invited_by_offense.erase(current_player_color);
				}
			}
		}
	}
}

void GameState::force_negotiation()
{
	assignments.offensive_encounter_card = CosmicCardType::Negotiate;
	assignments.defensive_encounter_card = CosmicCardType::Negotiate;

	setup_negotiation();
}

void GameState::setup_negotiation()
{
	//TODO: How to enforce a one minute timer?
	//Allies return ships to their colonies; similar logic to the Force Field artifact card
	for(auto i=assignments.offensive_allies.begin(),e=assignments.offensive_allies.end();i!=e;++i)
	{
		allies_to_be_stopped.insert(i->first);
	}

	for(auto i=assignments.defensive_allies.begin(),e=assignments.defensive_allies.end();i!=e;++i)
	{
		allies_to_be_stopped.insert(i->first);
	}
	stop_allies();
	//Return offensive ships to their colonies as well (does the timing ever matter here?)
	PlayerInfo &offense = get_player(assignments.get_offense());
	for(auto i=hyperspace_gate.begin(),e=hyperspace_gate.end();i!=e;++i)
	{
		if(*i == offense.color)
		{
			move_ship_to_colony(offense,hyperspace_gate);
		}
	}

	//TODO: Handle this situation better once the client/server aspect of the game is solidifed; for now we can accept input from 'both' players
	//Need to know how many cards will be exchanged by each player and if either (or both) players will receive a colony
	//Players cannot see each other's cards during a deal. They can say whatever they want about their hand and it does not have to be true or false.
	//Technically players can offer specific cards but if they do they *must* actually have the card. TODO: We could support this behavior at some point and enforce trading specific types of cards
	std::stringstream prompt("Was the deal successful?\n");
	std::vector<std::string> options;
	options.push_back("Y");
	options.push_back("N");
	unsigned response = prompt_player(assignments.get_offense(),prompt.str(),options); //FIXME: Prompt both the offense and defense here?

	if(response == 0)
	{
		deal_params.successful = true;
	}

	if(deal_params.successful) //Collect the terms of the deal
	{
		bool valid_deal = false;
		do
		{
			std::vector< std::pair<PlayerColors,unsigned> > defense_valid_colonies = get_valid_colonies(assignments.defense); //A list of planet colors and indices
			std::vector< std::pair<PlayerColors,unsigned> > offense_valid_colonies = get_valid_colonies(assignments.get_offense()); //A list of planet colors and indices

			if(defense_valid_colonies.empty())
			{
				server.broadcast_message("Note: The defense has no valid colonies and therefore cannot allow the offense to establish a colony!\n");
			}
			else
			{
				prompt.str(std::string());
				prompt << "Will the " << to_string(assignments.get_offense()) << " player (offense) establish a colony on any one planet where the " << to_string(assignments.defense) << " player (defense) has a colony?\n";
				//TODO: Prompt both the offense and defense and refuse to move on until they agree? That seems fairer
				response = prompt_player(assignments.get_offense(),prompt.str(),options);

				if(response == 0)
				{
					deal_params.offense_receives_colony = true;
				}
			}

			if(offense_valid_colonies.empty())
			{
				server.broadcast_message("Note: The offense has no valid colonies and therefore cannot allow the defense to establish a colony!\n");
			}
			else
			{
				prompt.str(std::string());
				prompt << "Will the " << to_string(assignments.defense) << " player (defense) establish a colony on any one planet where the " << to_string(assignments.get_offense()) << " player (offense) has a colony?\n";
				response = prompt_player(assignments.get_offense(),prompt.str(),options);

				if(response == 0)
				{
					deal_params.defense_receives_colony = true;
				}
			}

			prompt.str(std::string());
			prompt << "How many cards will the " << to_string(assignments.get_offense()) << " (offense) receive from the " << to_string(assignments.defense) << " player (defense)?\n";
			options.clear();
			options.push_back("0");
			for(unsigned i=0; i<get_player(assignments.defense).hand_size(); i++)
			{
				options.push_back(std::to_string(i+1));
			}
			deal_params.num_cards_to_offense = prompt_player(assignments.get_offense(),prompt.str(),options);

			if(deal_params.num_cards_to_offense > 0)
			{
				prompt.str(std::string());
				prompt << "Will these cards be chosen randomly? (If not they will be chosen by the " << to_string(assignments.defense) << " player (defense).\n";
				options.clear();
				options.push_back("Y");
				options.push_back("N");
				response = prompt_player(assignments.get_offense(),prompt.str(),options);

				if(response == 0)
				{
					deal_params.cards_to_offense_chosen_randomly = true;
				}
			}

			prompt.str(std::string());
			prompt << "How many cards will the " << to_string(assignments.defense) << " (defense) receive from the " << to_string(assignments.get_offense()) << " player (offense)?\n";
			options.clear();
			options.push_back("0");
			for(unsigned i=0; i<get_player(assignments.get_offense()).hand_size(); i++)
			{
				options.push_back(std::to_string(i+1));
			}
			deal_params.num_cards_to_defense = prompt_player(assignments.get_offense(),prompt.str(),options);

			if(deal_params.num_cards_to_defense > 0)
			{
				prompt.str(std::string());
				prompt << "Will these cards be chosen randomly? (If not they will be chosen by the " << to_string(assignments.get_offense()) << " player (offense).\n";
				options.clear();
				options.push_back("Y");
				options.push_back("N");
				response = prompt_player(assignments.get_offense(),prompt.str(),options);

				if(response == 0)
				{
					deal_params.cards_to_defense_chosen_randomly = true;
				}
			}

			valid_deal = (deal_params.offense_receives_colony || deal_params.defense_receives_colony || deal_params.num_cards_to_offense > 0 || deal_params.num_cards_to_defense > 0);
			if(!valid_deal)
			{
				server.broadcast_message("Error: Successful deal chosen but nothing was exchanged. Try again.\n");
			}
		} while(!valid_deal);
	}
}

void GameState::resolve_negotiation()
{
	//See if a player can quash the deal
	if(deal_params.successful)
	{
		//NOTE: This GameEvent is for just when the deal has been made, but not yet resolved
		GameEvent g(assignments.get_offense(),GameEventType::SuccessfulNegotiation); //NOTE: the color here is arbitrary
		resolve_game_event(g);
	}

	if(deal_params.successful)
	{
		//Now that we have the information we need, carry out the deal
		std::vector< std::pair<PlayerColors,unsigned> > defense_valid_colonies = get_valid_colonies(assignments.defense); //A list of planet colors and indices
		std::vector< std::pair<PlayerColors,unsigned> > offense_valid_colonies = get_valid_colonies(assignments.get_offense()); //A list of planet colors and indices
		if(deal_params.offense_receives_colony)
		{
			assert(!defense_valid_colonies.empty());
			//Choose from any of the valid defense colonies
			std::string msg("Choose a location to establish a new colony\n");
			server.send_message_to_client(assignments.get_offense(),msg);
			const std::pair<PlayerColors,unsigned> chosen_colony = prompt_valid_colonies(assignments.get_offense(),defense_valid_colonies);
			PlayerInfo &player_with_chosen_colony = get_player(chosen_colony.first);
			player_with_chosen_colony.planets.planet_push_back(chosen_colony.second,assignments.get_offense());
		}
		if(deal_params.defense_receives_colony)
		{
			assert(!offense_valid_colonies.empty());
			//Choose from any of the valid offense colonies
			std::string msg("Choose a location to establish a new colony\n");
			server.send_message_to_client(assignments.defense,msg);
			const std::pair<PlayerColors,unsigned> chosen_colony = prompt_valid_colonies(assignments.defense,offense_valid_colonies);
			PlayerInfo &player_with_chosen_colony = get_player(chosen_colony.first);
			player_with_chosen_colony.planets.planet_push_back(chosen_colony.second,assignments.defense);
		}
		std::vector<CosmicCardType> cards_to_offense;
		//Choose which cards will be taken from the defense
		PlayerInfo &defense = get_player(assignments.defense);
		if(deal_params.num_cards_to_offense > 0)
		{
			assert(deal_params.num_cards_to_offense <= defense.hand_size());
			if(deal_params.cards_to_offense_chosen_randomly)
			{
				for(unsigned i=0; i<deal_params.num_cards_to_offense; i++)
				{
					unsigned chosen_card_index = rand() % defense.hand_size();
					cards_to_offense.push_back(defense.hand_get(chosen_card_index));
					defense.hand_erase(defense.hand_begin()+chosen_card_index);
				}
			}
			else
			{
				std::stringstream prompt;
				prompt << "The " << to_string(assignments.defense) << " player has the following cards. Choose one to give to the offense.\n";
				for(unsigned i=0; i<deal_params.num_cards_to_offense; i++)
				{
					std::vector<std::string> options;
					for(unsigned i=0; i<defense.hand_size(); i++)
					{
						options.push_back(to_string(defense.hand_get(i)));
					}
					unsigned chosen_option = prompt_player(assignments.defense,prompt.str(),options);
					cards_to_offense.push_back(defense.hand_get(chosen_option));
					defense.hand_erase(defense.hand_begin()+chosen_option);
				}
			}
		}
		std::vector<CosmicCardType> cards_to_defense;
		//Choose which cards will be taken from the offense
		PlayerInfo &offense = get_player(assignments.get_offense());
		if(deal_params.num_cards_to_defense > 0)
		{
			assert(deal_params.num_cards_to_defense <= offense.hand_size());
			if(deal_params.cards_to_defense_chosen_randomly)
			{
				for(unsigned i=0; i<deal_params.num_cards_to_defense; i++)
				{
					unsigned chosen_card_index = rand() % offense.hand_size();
					cards_to_defense.push_back(offense.hand_get(chosen_card_index));
					offense.hand_erase(offense.hand_begin()+chosen_card_index);
				}
			}
			else
			{
				std::stringstream prompt;
				prompt << "The " << to_string(assignments.get_offense()) << " player has the following cards. Choose one to give to the defense.\n";
				for(unsigned i=0; i<deal_params.num_cards_to_defense; i++)
				{
					std::vector<std::string> options;
					for(unsigned i=0; i<offense.hand_size(); i++)
					{
						options.push_back(to_string(offense.hand_get(i)));
					}
					unsigned chosen_option = prompt_player(assignments.get_offense(),prompt.str(),options);
					cards_to_defense.push_back(offense.hand_get(chosen_option));
					offense.hand_erase(offense.hand_begin()+chosen_option);
				}
			}
		}
		//Distribute the chosen cards to their new players
		for(auto i=cards_to_offense.begin(),e=cards_to_offense.end();i!=e;++i)
		{
			offense.hand_push_back(*i);
		}
		for(auto i=cards_to_defense.begin(),e=cards_to_defense.end();i!=e;++i)
		{
			defense.hand_push_back(*i);
		}

		//The deal was actually successful, so we use a separate GameEventType for Tick-Tock triggers
		GameEvent g(assignments.get_offense(),GameEventType::SuccessfulDeal); //Color is arbitrary here
		resolve_game_event(g);

		assignments.successful_encounter = true;
	}
	else //Unsuccessful deal, each player loses 3 ships to the warp
	{
		//NOTE: This signifies an unsuccessful encounter
		//This logic should be similar to part of a plague
		lose_ships_to_warp(assignments.get_offense(),3);
		lose_ships_to_warp(assignments.defense,3);
	}
}

void GameState::setup_compensation()
{
	//The player with the negotiate loses, but collects compensation later
	if(assignments.offensive_encounter_card == CosmicCardType::Negotiate)
	{
		assignments.player_receiving_compensation = assignments.get_offense();
		assignments.player_giving_compensation = assignments.defense;
	}
	else
	{
		assignments.player_receiving_compensation = assignments.defense;
		assignments.player_giving_compensation = assignments.get_offense();
	}

	std::stringstream announce;
	announce << "The " << to_string(assignments.player_receiving_compensation) << " player has lost the encountner, but will receive compensation from the " << to_string(assignments.player_giving_compensation) << " player.\n";
	server.broadcast_message(announce.str());
}

void GameState::resolve_compensation()
{
	//The losing player's ships go to the warp
	unsigned number_of_ships_lost = 0;

	if(assignments.player_receiving_compensation == assignments.get_offense())
	{
		for(auto i=hyperspace_gate.begin(),e=hyperspace_gate.end();i!=e;++i)
		{
			if(*i == assignments.player_receiving_compensation)
			{
				number_of_ships_lost++;
			}
		}
		defense_win_resolution();
	}
	else
	{
		const PlanetInfo &encounter_planet = get_player(assignments.planet_location).planets.get_planet(assignments.planet_id);
		for(auto i=encounter_planet.begin(),e=encounter_planet.end();i!=e;++i)
		{
			if(*i == assignments.player_receiving_compensation)
			{
				number_of_ships_lost++;
			}
		}
		offense_win_resolution();
	}

	if(!assignments.stop_compensation_and_rewards)
	{
		//For each ship lost, the losing player receives a card from the opponent's hand chosen at random
		PlayerInfo &player_giving_compensation = get_player(assignments.player_giving_compensation);
		PlayerInfo &player_receiving_compensation = get_player(assignments.player_receiving_compensation);
		std::stringstream announce;
		announce << "The " << to_string(player_receiving_compensation.color) << " player will receive up to " << number_of_ships_lost << " cards from the " << to_string(player_giving_compensation.color) << " player as compensation\n";
		server.broadcast_message(announce.str());
		for(unsigned i=0; i<number_of_ships_lost; i++)
		{
			if(player_giving_compensation.hand_empty())
				break;

			unsigned card_choice = rand() % player_giving_compensation.hand_size();
			player_receiving_compensation.hand_push_back(player_giving_compensation.hand_get(card_choice));
			player_giving_compensation.hand_erase(player_giving_compensation.hand_begin()+card_choice);
		}
	}
}

void GameState::setup_attack()
{
	//Base values from encounter cards
	if(assignments.offensive_encounter_card == CosmicCardType::Morph && assignments.defensive_encounter_card == CosmicCardType::Morph)
	{
		//There's only one Morph card in the deck, but the rules explicitly state that *both* players lose the encounter should this scenario somehow occur
		server.broadcast_message("Both players revealed a Morph card! Both players lose the encounter and all ships involved will go to the warp. Yes, this scenario is explicitly laid out in the rules.\n");
		return;
	}
	else if(assignments.offensive_encounter_card == CosmicCardType::Morph)
	{
		assignments.defense_attack_value = static_cast<unsigned>(assignments.defensive_encounter_card);
		assignments.offense_attack_value = assignments.defense_attack_value;
	}
	else if(assignments.defensive_encounter_card == CosmicCardType::Morph)
	{
		assignments.offense_attack_value = static_cast<unsigned>(assignments.offensive_encounter_card);
		assignments.defense_attack_value = assignments.offense_attack_value;
	}
	else
	{
		assignments.offense_attack_value = static_cast<unsigned>(assignments.offensive_encounter_card);
		assignments.defense_attack_value = static_cast<unsigned>(assignments.defensive_encounter_card);
	}

	std::stringstream announce;
	announce << "Initial scores from encounter cards: Offense = " << assignments.offense_attack_value << "; Defense = " << assignments.defense_attack_value << "\n";
	server.broadcast_message(announce.str());

	//Add score from ships
	announce.str("");
	for(auto i=hyperspace_gate.begin(),e=hyperspace_gate.end(); i!=e; ++i)
	{
		if(*i == assignments.get_offense() || assignments.offensive_allies.find(*i) != assignments.offensive_allies.end())
		{
			announce << "Adding " << to_string(*i) << " ship from hyperspace gate to offense score.\n";
			assignments.offense_attack_value++;
		}
	}

	const PlanetInfo &encounter_planet = get_player(assignments.planet_location).planets.get_planet(assignments.planet_id);
	for(auto i=encounter_planet.begin(),e=encounter_planet.end();i!=e;++i)
	{
		if(*i == assignments.defense)
		{
			announce << "Adding " << to_string(*i) << " ship from the colony being attacked to defense score.\n";
			assignments.defense_attack_value++;
		}
	}
	for(auto i=assignments.defensive_allies.begin(),e=assignments.defensive_allies.end();i!=e;++i)
	{
		announce << "Adding " << i->second << " ship(s) from the " << to_string(i->first) << " defensive ally.\n";
		assignments.defense_attack_value += i->second;
	}

	announce << "Total score after adding ships (ties go to the defense): Offense = " << assignments.offense_attack_value << "; Defense = " << assignments.defense_attack_value << "\n";
	server.broadcast_message(announce.str());
}

void GameState::offense_win_resolution()
{
	//Defense and defensive ally ships go to the warp
	PlanetInfo &encounter_planet = get_player(assignments.planet_location).planets.get_planet(assignments.planet_id);
	for(auto i=encounter_planet.begin();i!=encounter_planet.end();)
	{
		if(*i == assignments.defense)
		{
			warp.push_back(std::make_pair(*i,encounter_num));
			i = get_player(assignments.planet_location).planets.planet_erase(assignments.planet_id,i); //FIXME: This should work but it's pretty ugly...
		}
		else
		{
			++i;
		}
	}
	for(auto i=defensive_ally_ships.begin();i!=defensive_ally_ships.end();)
	{
		warp.push_back(std::make_pair(*i,encounter_num));
		i=defensive_ally_ships.erase(i);
	}

	//Offense and offensive ally ships create (or reinforce) a colony on the encounter planet
	for(auto i=hyperspace_gate.begin();i!=hyperspace_gate.end();)
	{
		get_player(assignments.planet_location).planets.planet_push_back(assignments.planet_id,*i); //FIXME: Ugly here too
		i=hyperspace_gate.erase(i);
	}

	assignments.successful_encounter = true;
}

void GameState::defense_win_resolution()
{
	//Offense and offensive ally ships go to the warp
	for(auto i=hyperspace_gate.begin();i!=hyperspace_gate.end();)
	{
		warp.push_back(std::make_pair(*i,encounter_num));
		i=hyperspace_gate.erase(i);
	}
	//Defensive allies return each of their ships to one of their colonies
	while(!defensive_ally_ships.empty())
	{
		move_ship_to_colony(get_player(*defensive_ally_ships.begin()),defensive_ally_ships);
	}
	//Handle defender rewards (Note: This should generate quite a few Remora triggers)
	if(!assignments.stop_compensation_and_rewards)
	{
		for(auto i=assignments.defensive_allies.begin(),e=assignments.defensive_allies.end();i!=e;++i)
		{
			unsigned num_ships = i->second;
			for(unsigned ship=0; ship<num_ships; ship++)
			{
				//For each ship, the ally gets the choice of returning a ship from the warp or drawing a card
				bool ship_exists_in_warp = false;
				for(auto ii=warp.begin(),ee=warp.end();ii!=ee;++ii)
				{
					if(ii->first == i->first)
					{
						ship_exists_in_warp = true;
					}
				}

				if(!ship_exists_in_warp) //The player has no ships in the warp and must draw a card
				{
					std::stringstream announce;
					announce << "Defender rewards. Drawing a card for the " << to_string(i->first) << " player.\n";
					server.broadcast_message(announce.str());
					draw_cosmic_card(get_player(i->first));
				}
				else
				{
					std::string prompt("Choose which defender reward you would like to receieve.\n");
					std::vector<std::string> options;
					options.push_back("Retrieve one of your ships from the warp.");
					options.push_back("Draw a card from the Cosmic deck.");
					unsigned choice = prompt_player(i->first,prompt,options);

					if(choice == 0)
					{
						move_ship_from_warp_to_colony(get_player(i->first));
					}
					else
					{
						draw_cosmic_card(get_player(i->first));
					}
				}
			}
		}
	}

	GameEvent g(assignments.defense,GameEventType::DefensiveEncounterWin);
	resolve_game_event(g);
}

void GameState::resolve_attack()
{
	if(assignments.offensive_encounter_card == CosmicCardType::Morph && assignments.defensive_encounter_card == CosmicCardType::Morph)
	{
		//Send all ships involved to the warp
		for(auto i=hyperspace_gate.begin();i!=hyperspace_gate.end();)
		{
			warp.push_back(std::make_pair(*i,encounter_num));
			i=hyperspace_gate.erase(i);
		}
		PlanetInfo &encounter_planet = get_player(assignments.planet_location).planets.get_planet(assignments.planet_id);
		for(auto i=encounter_planet.begin();i!=encounter_planet.end();)
		{
			if(*i == assignments.defense)
			{
				warp.push_back(std::make_pair(*i,encounter_num));
				get_player(assignments.planet_location).planets.planet_erase(assignments.planet_id,i);
			}
			else
			{
				++i;
			}
		}
		for(auto i=defensive_ally_ships.begin();i!=defensive_ally_ships.end();)
		{
			warp.push_back(std::make_pair(*i,encounter_num));
			i=defensive_ally_ships.erase(i);
		}

		return;
	}

	if(assignments.offense_attack_value > assignments.defense_attack_value)
	{
		offense_win_resolution();
	}
	else //Ties go to the defense
	{
		defense_win_resolution();
	}
}

void GameState::add_reinforcements(const PlayerColors player, const unsigned value)
{
	bool player_is_offense;
	if(player == assignments.get_offense() || assignments.offensive_allies.find(player) != assignments.offensive_allies.end())
	{
		player_is_offense = true;
	}
	else if(player == assignments.defense || assignments.defensive_allies.find(player) != assignments.defensive_allies.end())
	{
		player_is_offense = false;
	}
	else
	{
		assert(0 && "Invalid player casted reinforcement card!");
	}

	std::stringstream prompt;
	prompt << "Would you like to add reinforcements to the offense or defense? (You are fighting with the ";
	if(player_is_offense)
	{
		prompt << "offense)\n";
	}
	else
	{
		prompt << "defense)\n";
	}
	std::vector<std::string> options;
	options.push_back("Offense");
	options.push_back("Defense");

	unsigned choice = prompt_player(player,prompt.str(),options);

	if(choice == 0)
	{
		assignments.offense_attack_value += value;
	}
	else if(choice == 1)
	{
		assignments.defense_attack_value += value;
	}

	std::stringstream announce;
	announce << "After reinforcements, the revised score is (ties go to the defense): Offense = " << assignments.offense_attack_value << "; Defense = " << assignments.defense_attack_value << "\n";
	server.broadcast_message(announce.str());
}

void GameState::human_encounter_win_condition()
{

	server.broadcast_message("The human was zapped! Their side will automatically win the encounter!\n");
	assignments.human_wins_encounter = true;
}

void GameState::resolve_human_encounter_win()
{
	bool human_on_offense = false;
	if(get_player(assignments.get_offense()).alien->get_name().compare("Human") == 0)
	{
		human_on_offense = true;
	}
	for(auto i=assignments.offensive_allies.begin(),e=assignments.offensive_allies.end();i!=e;++i)
	{
		if(get_player(i->first).alien->get_name().compare("Human") == 0)
		{
			human_on_offense = true;
		}
	}

	if(human_on_offense)
	{
		offense_win_resolution();
	}
	else
	{
		defense_win_resolution();
	}
}

void GameState::start_game()
{
	execute_turn();

	dump(); //Dump after each turn for sanity
	encounter_num++;

	while(1)
	{
		bool go_to_next_player;
		std::stringstream announce;
		if(assignments.successful_encounter && !is_second_encounter_for_offense)
		{
			//The offense has the option of having a second encounter
			std::string prompt("The offense just had their first successful encounter of the turn. Would they like to have a second encounter?\n");
			std::vector<std::string> options;
			options.push_back("Y");
			options.push_back("N");
			unsigned response = prompt_player(assignments.get_offense(),prompt,options);

			if(response == 0)
			{
				announce << "The offense has elected to have a second encounter this turn.\n";
				is_second_encounter_for_offense = true;
				go_to_next_player = false;
			}
			else
			{
				announce << "The offense has declined a second encounter on their turn. Play proceeds to the next player.\n";
				go_to_next_player = true;
			}
		}
		else if(assignments.successful_encounter)
		{
			announce << "The offense has won their second encounter of their turn. Play proceeds to the next player.\n";
			is_second_encounter_for_offense = false;
			go_to_next_player = true;
		}
		else
		{
			announce << "The offense did not win the last encounter. Play proceeds to the next player.\n";
			is_second_encounter_for_offense = false;
			go_to_next_player = true;
		}
		server.broadcast_message(announce.str());

		const PlayerColors last_offense = assignments.get_offense();
		assignments.clear();
		deal_params.clear();
		for(unsigned i=0; i<players.size(); i++)
		{
			players[i].current_role = EncounterRole::None;
		}

		if(go_to_next_player)
		{
			unsigned next_player_index = max_player_sentinel;
			for(unsigned i=0; i<players.size();i++)
			{
				if(players[i].color == last_offense)
				{
					next_player_index = (i+1) % players.size();
				}
			}
			assert(next_player_index != max_player_sentinel);
			assignments.set_offense(players[next_player_index].color);
		}
		else
		{
			assignments.set_offense(last_offense);
		}

		execute_turn();

		dump(); //Dump after each turn for sanity
		encounter_num++;
	}
}

void GameState::update_turn_phase(const TurnPhase phase)
{
	state = phase;
	//Update client GUIs
	std::string msg("[turn_phase] ");
	msg.append(to_string(phase));
	server.broadcast_message(msg);
}

void GameState::execute_turn()
{
	//Start Turn Phase
	update_turn_phase(TurnPhase::StartTurn);
	for(unsigned i=0; i<players.size(); i++)
	{
		players[i].current_role = EncounterRole::None;
	}

	//Ensure the offense has a valid hand
	PlayerInfo &offense = get_player(assignments.get_offense());
	offense.current_role = EncounterRole::Offense;
	bool offense_needs_discard = !offense.has_encounter_cards_in_hand();

	std::stringstream announce;
	announce << "The " << to_string(assignments.get_offense()) << " Player is now the offense.\n";
	server.broadcast_message(announce.str());
	if(offense_needs_discard)
	{
		announce.str("");
		announce << "The offense has no encounter cards in hand. They now must discard their hand and draw eight cards\n";
		server.broadcast_message(announce.str());
		discard_and_draw_new_hand(offense);
	}

	check_for_game_events();

	//Regroup Phase
	update_turn_phase(TurnPhase::Regroup);

	//If the offense has any ships in the warp, they retrieve one of them
	for(auto i=warp.begin(),e=warp.end();i!=e;++i)
	{
		if(i->first == offense.color)
		{
			announce.str("");
			announce << "The " << to_string(assignments.get_offense()) << " player will now regroup\n";
			server.broadcast_message(announce.str());
			move_ship_from_warp_to_colony(offense);
			break;
		}
	}

	check_for_game_events();

	//Destiny Phase
	update_turn_phase(TurnPhase::Destiny);

	draw_from_destiny_deck();

	check_for_game_events();

	//Launch Phase
	update_turn_phase(TurnPhase::Launch);

	choose_opponent_planet();

	send_in_ships(assignments.get_offense());

	check_for_game_events();

	//Alliance Phase
	update_turn_phase(TurnPhase::Alliance);

	//Offense invites, then the defense invites, then players accept/reject and commit ships in turn order
	std::set<PlayerColors> potential_allies;
	for(unsigned i=0; i<players.size(); i++)
	{
		if((players[i].color != assignments.get_offense()) && (players[i].color != assignments.defense))
		{
			potential_allies.insert(players[i].color);
		}
	}

	std::set<PlayerColors> invited_by_offense = invite_allies(potential_allies,true);
	std::set<PlayerColors> invited_by_defense = invite_allies(potential_allies,false);

	form_alliances(invited_by_offense,invited_by_defense);

	check_for_game_events();

	//Planning Phase
	update_turn_phase(TurnPhase::Planning_before_selection);

	//If the offense happens to have no encounter cards in hand, the turn ends immediately
	if(!offense.has_encounter_cards_in_hand())
	{
		//Return ships from the hyperspace gate to their players' colonies
		for(auto i=hyperspace_gate.begin(),e=hyperspace_gate.end();i!=e;++i)
		{
			move_ship_to_colony(get_player(*i),hyperspace_gate);
		}
		//Return ships for any defensive allies as well
		for(auto i=defensive_ally_ships.begin(),e=defensive_ally_ships.end();i!=e;++i)
		{
			move_ship_to_colony(get_player(*i),defensive_ally_ships);
		}

		return;
	}

	//If the defense has no encounter cards in hand, they get to draw
	PlayerInfo &defense = get_player(assignments.defense);
	if(!defense.has_encounter_cards_in_hand())
	{
		discard_and_draw_new_hand(defense);
	}

	//Before cards are selected effects can now resolve
	check_for_game_events();

	std::string prompt("Which encounter card would you like to play?\n");
	std::vector<std::string> options;
	for(auto i=offense.hand_begin(),e=offense.hand_end();i!=e;++i)
	{
		if(static_cast<unsigned>(*i) <= static_cast<unsigned>(CosmicCardType::Morph))
		{
			options.push_back(to_string(*i));
		}
	}
	unsigned response = prompt_player(assignments.get_offense(),prompt,options);

	unsigned option = 0;
	for(auto i=offense.hand_begin(),e=offense.hand_end();i!=e;++i)
	{
		if(static_cast<unsigned>(*i) <= static_cast<unsigned>(CosmicCardType::Morph))
		{
			if(option == response)
			{
				assignments.offensive_encounter_card = *i;
				cosmic_discard.push_back(*i);
				offense.hand_erase(i);
				break;
			}
			option++;
		}
	}

	options.clear();
	for(auto i=defense.hand_begin(),e=defense.hand_end();i!=e;++i)
	{
		if(static_cast<unsigned>(*i) <= static_cast<unsigned>(CosmicCardType::Morph))
		{
			options.push_back(to_string(*i));
		}
	}
	response = prompt_player(assignments.defense,prompt,options);

	option = 0;
	for(auto i=defense.hand_begin(),e=defense.hand_end();i!=e;++i)
	{
		if(static_cast<unsigned>(*i) <= static_cast<unsigned>(CosmicCardType::Morph))
		{
			if(option == response)
			{
				assignments.defensive_encounter_card = *i;
				cosmic_discard.push_back(*i);
				defense.hand_erase(i);
				break;
			}
			option++;
		}
	}

	assert(assignments.offensive_encounter_card != CosmicCardType::None && "Failed to obtain offensive encounter card!");
	assert(assignments.defensive_encounter_card != CosmicCardType::None && "Failed to obtain defensive encounter card!");

	//After cards are selected effects can now resolve
	update_turn_phase(TurnPhase::Planning_after_selection);
	check_for_game_events();

	//Reveal Phase
	update_turn_phase(TurnPhase::Reveal);

	announce.str("");
	announce << "The offense has encounter card: " << to_string(assignments.offensive_encounter_card) << "\n";
	announce << "The defense has encounter card: " << to_string(assignments.defensive_encounter_card) << "\n";
	server.broadcast_message(announce.str());

	//Good primer on negotiation specifics: https://boardgamegeek.com/thread/1212948/question-about-trading-cards-during-negotiate/page/1
	if(assignments.offensive_encounter_card == CosmicCardType::Negotiate && assignments.defensive_encounter_card == CosmicCardType::Negotiate)
	{
		setup_negotiation();
	}
	else if(assignments.offensive_encounter_card == CosmicCardType::Negotiate || assignments.defensive_encounter_card == CosmicCardType::Negotiate)
	{
		setup_compensation();
	}
	else
	{
		//Both players played attack/morph cards, time to do math
		setup_attack();
	}

	check_for_game_events();

	update_turn_phase(TurnPhase::Resolution);

	//NOTE: It's easier to implement artifacts in a way that we check before game events before we carry out resolution tasks. If any future Aliens require different behavior we'll have to revisit this decision
	check_for_game_events();

	if(assignments.human_wins_encounter)
	{
		resolve_human_encounter_win();
	}
	else if(assignments.offensive_encounter_card == CosmicCardType::Negotiate && assignments.defensive_encounter_card == CosmicCardType::Negotiate)
	{
		resolve_negotiation();
	}
	else if(assignments.offensive_encounter_card == CosmicCardType::Negotiate || assignments.defensive_encounter_card == CosmicCardType::Negotiate)
	{
		resolve_compensation();
	}
	else
	{
		resolve_attack();
	}

	//Clean ups
	update_player_scores();

	//If an Alien was zapped this turn it can now be used again
	for(unsigned i=0; i<players.size(); i++)
	{
		players[i].alien_zapped = false;
	}
}

void GameState::swap_encounter_cards()
{
	CosmicCardType tmp = assignments.offensive_encounter_card;
	assignments.offensive_encounter_card = assignments.defensive_encounter_card;
	assignments.defensive_encounter_card = tmp;
}

void GameState::swap_main_player_hands()
{
	PlayerInfo &offense = get_player(assignments.get_offense());
	PlayerInfo &defense = get_player(assignments.defense);

	std::vector<CosmicCardType> tmp = offense.get_hand_data();
	offense.set_hand_data(defense.get_hand_data());
	defense.set_hand_data(tmp);
}

std::vector< std::pair<PlayerColors,unsigned> > GameState::get_valid_colonies(const PlayerColors color) const
{
	std::vector< std::pair<PlayerColors,unsigned> > valid_colonies; //A list of planet colors and indices

	for(auto player_begin=players.begin(),player_end=players.end();player_begin!=player_end;++player_begin)
	{
		for(unsigned planet=0; planet<player_begin->planets.size(); planet++)
		{
			for(unsigned ships=0; ships<player_begin->planets.planet_size(planet); ships++)
			{
				if(player_begin->planets.get_ship(planet,ships) == color) //If this ship matches our color we have a colony there
				{
					valid_colonies.push_back(std::make_pair(player_begin->color,planet));
					break;
				}
			}
		}
	}

	return valid_colonies;
}

const std::pair<PlayerColors,unsigned> GameState::prompt_valid_colonies(const PlayerColors color, const std::vector< std::pair<PlayerColors,unsigned> > &valid_colonies)
{
	std::stringstream prompt;
	prompt << "The " << to_string(color) << " player has the following valid colonies to choose from:\n";
	std::vector<std::string> options;
	for(unsigned i=0; i<valid_colonies.size(); i++)
	{
		std::stringstream opt;
		opt << to_string(valid_colonies[i].first) << " Planet " << valid_colonies[i].second;
		options.push_back(opt.str());
	}
	unsigned chosen_option = prompt_player(color,prompt.str(),options);

	return valid_colonies[chosen_option];
}

bool GameState::player_has_ship_in_warp_from_prior_encounter(const PlayerColors player) const
{
	for(auto i=warp.cbegin(),e=warp.cend();i!=e;++i)
	{
		if((i->first == player) && (i->second < encounter_num))
		{
			return true;
		}
	}

	return false;
}

//Source = hyperspace_gate, defensive_ally_ships, etc. but not the warp (see move_ship_from_warp_to_colony instead)
void GameState::move_ship_to_colony(PlayerInfo &p, PlanetInfoFull<PlayerColors> &source)
{
	//Check that at least one ship of the specified color resides in the source; if not, return
	bool ship_exists_in_source = false;
	for(auto i=source.begin(),e=source.end();i!=e;++i)
	{
		if(*i == p.color)
		{
			ship_exists_in_source = true;
		}
	}

	if(!ship_exists_in_source)
		return;

	//Gather the valid options and present them to the player
	std::vector< std::pair<PlayerColors,unsigned> > valid_colonies = get_valid_colonies(p.color); //A list of planet colors and indices

	if(!valid_colonies.size())
	{
		if(p.color == assignments.get_offense())
		{
			std::stringstream announce;
			announce << "The  " << to_string(p.color) << " player has no valid colonies! Placing the ship directly on the hyperspace gate.\n";
			server.broadcast_message(announce.str());
			hyperspace_gate.push_back(p.color);
		}
		else
		{
			//No colony to return the ship to!
			//FIXME: What if a player has one remaining ship on one colony and then used it to ally and then a force field was played? Where does the ship go?
			return;
		}
	}
	else
	{
		std::string msg("Choose a colony to return a ship to.\n");
		server.send_message_to_client(p.color,msg);
		const std::pair<PlayerColors,unsigned> chosen_colony = prompt_valid_colonies(p.color,valid_colonies);

		//Now actually add the colony
		bool colony_found = false; //paranoia
		for(auto player_begin=players.begin(),player_end=players.end();player_begin!=player_end;++player_begin)
		{
			if(player_begin->color != chosen_colony.first)
				continue;
			PlanetInfo &chosen_planet = player_begin->planets.get_planet(chosen_colony.second);
			for(auto i=chosen_planet.begin(),e=chosen_planet.end();i!=e; ++i)
			{
				if(*i == p.color)
				{
					colony_found = true;
					break;
				}
			}
			if(colony_found)
			{
				player_begin->planets.planet_push_back(chosen_colony.second,p.color);
				break;
			}
		}

		assert(colony_found && "Failed to find colony to place ship!");
	}

	//Remove the ship from the source
	for(auto i=source.begin(),e=source.end();i!=e;++i)
	{
		if(*i == p.color)
		{
			source.erase(i);
			break;
		}
	}
}

void GameState::move_ship_from_warp_to_colony(PlayerInfo &p)
{
	//Check that at least one ship of the specified color resides in the source; if not, return
	bool ship_exists_in_source = false;
	for(auto i=warp.begin(),e=warp.end();i!=e;++i)
	{
		if(i->first == p.color)
		{
			ship_exists_in_source = true;
		}
	}

	if(!ship_exists_in_source)
		return;

	//Gather the valid options and present them to the player
	std::vector< std::pair<PlayerColors,unsigned> > valid_colonies = get_valid_colonies(p.color); //A list of planet colors and indices

	if(!valid_colonies.size())
	{
		if(p.color == assignments.get_offense())
		{
			std::stringstream announce;
			announce << "The  " << to_string(p.color) << " player has no valid colonies! Placing the ship directly on the hyperspace gate.\n";
			server.broadcast_message(announce.str());
			hyperspace_gate.push_back(p.color);
		}
		else
		{
			//No colony to return the ship to!
			return;
		}
	}
	else
	{
		std::string msg("Choose a colony to return a ship to.\n");
		server.send_message_to_client(p.color,msg);
		const std::pair<PlayerColors,unsigned> chosen_colony = prompt_valid_colonies(p.color,valid_colonies);

		//Now actually add the colony
		bool colony_found = false; //paranoia
		for(auto player_begin=players.begin(),player_end=players.end();player_begin!=player_end;++player_begin)
		{
			if(player_begin->color != chosen_colony.first)
				continue;
			PlanetInfo &chosen_planet = player_begin->planets.get_planet(chosen_colony.second);
			for(auto i=chosen_planet.begin(),e=chosen_planet.end();i!=e; ++i)
			{
				if(*i == p.color)
				{
					colony_found = true;
					break;
				}
			}
			if(colony_found)
			{
				player_begin->planets.planet_push_back(chosen_colony.second,p.color);
				break;
			}
		}

		assert(colony_found && "Failed to find colony to place ship!");
	}

	//Remove the ship from the source
	for(auto i=warp.begin(),e=warp.end();i!=e;++i)
	{
		if(i->first == p.color)
		{
			warp.erase(i);
			break;
		}
	}

	GameEvent g(p.color,GameEventType::RetrieveWarpShip);
	resolve_game_event(g);
}

//When an Alien is hit by a CosmicZap, it is disabled for the rest of the encounter
void GameState::zap_alien(const PlayerColors player)
{
	PlayerInfo &p = get_player(player);
	p.alien_zapped = true;
}

const std::string GameState::get_cosmic_discard() const
{
	std::stringstream ret;
	ret << "Cosmic discard pile: {";
	for(auto i=cosmic_discard.begin(),e=cosmic_discard.end();i!=e;++i)
	{
		if(i != cosmic_discard.begin())
			ret << ",";
		ret << to_string(*i);
	}
	ret << "}\n";

	return ret.str();
}

const std::string GameState::get_destiny_discard() const
{
	return destiny_deck.get_discard();
}

unsigned GameState::prompt_player(const PlayerColors player, const std::string &prompt, const std::vector<std::string> &options) const
{
	std::string response;
	unsigned choice;
	while(1)
	{
		std::stringstream outgoing;
		outgoing << prompt;
		for(unsigned i=0; i<options.size(); i++)
		{
			outgoing << i << ": " << options[i] << "\n";
		}
		std::cout << "Waiting on response from the " << to_string(player) << " player..." << std::flush;

		std::string message("[needs_response] Please choose one of the above options.\n");
		outgoing << message;
		server.send_message_to_client(player,outgoing.str());
		response = server.receive_message_from_client(player);

		if(response.size() && is_only_digits(response))
		{
			choice = std::stoi(response);
			if(choice < options.size())
			{
				break;
			}
		}

		//Check for valid player commands
		//FIXME: These are now deprecated
		if(response.compare("info hand") == 0)
		{
			const std::string player_hand = get_player_const(player).get_hand();
			server.send_message_to_client(player,player_hand);
		}
		else if(response.compare("info discard cosmic") == 0)
		{
			const std::string cosmic_discard_pile = get_cosmic_discard();
			server.send_message_to_client(player,cosmic_discard_pile);
		}
		else if(response.compare("info discard destiny") == 0)
		{
			const std::string destiny_discard_pile = get_destiny_discard();
			server.send_message_to_client(player,destiny_discard_pile);
		}
		else if(response.compare("info board") == 0)
		{
			const std::string game_board = get_game_board();
			server.send_message_to_client(player,game_board);
		}
		else if(response.compare("info alien") == 0)
		{
			const std::string alien_desc = get_player_const(player).get_alien_desc();
			server.send_message_to_client(player,alien_desc);
		}
		else if(response.compare("info aliens") == 0)
		{
			std::stringstream msg;
			for(unsigned i=0; i<players.size(); i++)
			{
				//Only show the aliens that have been revealed!
				if(get_player_const(players[i].color).alien_revealed())
				{
					msg << "Revealed alien for the " << to_string(players[i].color) << " player:\n";
					msg << get_player_const(players[i].color).get_alien_desc();
				}
			}
			if(msg.str().empty())
			{
				msg << "No aliens have been revealed yet.\n";
			}
			server.send_message_to_client(player,msg.str());
		}
	}

	return choice;
}

void GameState::dump_current_stack() const
{
	std::stack<GameEvent> copy_stack = stack;

	std::stringstream announce;
	announce << "Current game stack:\n";
	while(!copy_stack.empty())
	{
		GameEvent g = copy_stack.top();
		unsigned depth = copy_stack.size()-1;
		announce << "{" << depth << ": " << to_string(g.player) << " -> " << to_string(g.event_type) << "}\n";
		copy_stack.pop();
	}
	server.broadcast_message(announce.str());
	assert(copy_stack.empty() && "Error printing stack!");
}

std::vector<PlayerColors> GameState::get_player_order()
{
	//Enforce player order during resolution
	unsigned player_index = max_player_sentinel;
	for(unsigned i=0; i<players.size(); i++)
	{
		if(players[i].color == assignments.get_offense())
		{
			player_index = i;
			break;
		}
	}

	std::vector<PlayerColors> player_order;
	player_order.push_back(assignments.get_offense()); //Offense goes first
	if(assignments.defense != PlayerColors::Invalid)
	{
		player_order.push_back(assignments.defense); //Then defense, if the defense has been assigned
	}
	//Then we proceed clockwise (aka in turn order) from the offense
	while(player_order.size() < players.size())
	{
		player_index = (player_index+1) % players.size();
		//If the current player isn't already accounted for, add them to the list
		if(std::find(player_order.begin(),player_order.end(),players[player_index].color) == player_order.end())
		{
			player_order.push_back(players[player_index].color);
		}
	}

	std::set<PlayerColors> player_order_check(player_order.begin(),player_order.end());

	if(player_order.size() != player_order_check.size()) //Extra info in the event the check below fails
	{
		std::cout << "Player order: {";
		for(auto i=player_order.begin(),e=player_order.end();i!=e;++i)
		{
			if(i != player_order.begin())
			{
				std::cout << ",";
			}
			std::cout << to_string(*i);
		}
		std::cout << "}\n";

		std::cout << "Player order check: {";
		for(auto i=player_order_check.begin(),e=player_order_check.end();i!=e;++i)
		{
			if(i != player_order_check.begin())
			{
				std::cout << ",";
			}
			std::cout << to_string(*i);
		}
		std::cout << "}\n";
	}
	assert(player_order.size() == player_order_check.size() && "Error in determining player order!"); //Ensure the player_order vector elements are unique

	return player_order;
}

void GameState::resolve_game_event(const GameEvent g)
{
	stack.push(g);
	dump_current_stack();

	std::vector<PlayerColors> player_order = get_player_order();

	for(unsigned current_player_index=0; current_player_index<players.size(); current_player_index++)
	{
		PlayerInfo &current_player = get_player(player_order[current_player_index]);
		std::vector<GameEvent> valid_plays = current_player.can_respond(state,g);
		//NOTE: As of right now there's never a need to actually respond to an event more than once (even with reinforcements you can cast one and then respond to that one)
		//In general this property probably isn't true so this if stmt should eventually become a while stmt
		//If we do make that change, be sure to recalculate valid_plays after taking an action
		if(valid_plays.size()) //If there is a valid response...
		{
			bool take_action = false;
			GameEvent can_respond(current_player.color,GameEventType::None); //Arbitrary initialization value
			for(unsigned i=0; i<valid_plays.size(); i++)
			{
				if(valid_plays[i].event_type == GameEventType::AlienPower && current_player.alien->get_mandatory())
				{
					//If an alien power is mandatory we'll automatically respond with it
					can_respond = valid_plays[i];
					take_action = true;
					break;
				}
			}

			if(take_action)
			{
				std::stringstream announce;
				announce << "The " << to_string(current_player.color) << " player must respond to the " << to_string(g.event_type) << " action with their alien power\n";
				server.broadcast_message(announce.str());
			}
			else //If we haven't already forced the alien power play
			{
				std::stringstream prompt;
				prompt << "The " << to_string(current_player.color) << " player can respond to the " << to_string(g.event_type) << " action.\n";
				std::vector<std::string> options;
				for(unsigned i=0; i<valid_plays.size(); i++)
				{
					options.push_back(to_string(valid_plays[i].event_type));
				}
				std::stringstream opt;
				opt << "None (Resolve " << to_string(g.event_type) << ")";
				options.push_back(opt.str());

				unsigned chosen_option = prompt_player(current_player.color,prompt.str(),options);

				if(chosen_option < valid_plays.size())
				{
					take_action = true;
					can_respond = valid_plays[chosen_option];
				}
			}

			if(take_action)
			{
				if(can_respond.callback_if_action_taken)
				{
					can_respond.callback_if_action_taken();
				}
				resolve_game_event(can_respond);

				//This action resolved and will counter the next item in the stack, which means there's nothing for the other players to actually respond to
				//For example:
				//
				//Current game stack:
				//1: Yellow -> Cosmic Zap
				//0: Blue -> Alien Power
				//The Red player can respond to the Alien Power action.
				//Would you like to respond to the Alien Power with your Cosmic Zap? y/n
				//Red's response here is meaningless, and the break below prevents this prompt.
				//You could imagine a similar stack with one more level: Alien Power <- CosmicZap <- CardZap with another player with a CardZap behind
				//Assume this player does not want to CardZap the other CardZap that's already on the stack. They shouldn't be prompted to CardZap the CosmicZap after the CardZap on the stack resolves, because it's the CosmicZap has already been zapped (but hasn't actually been countered yet)
				if(invalidate_next_callback && (can_respond.event_type == GameEventType::CosmicZap || can_respond.event_type == GameEventType::CardZap))
				{
					break;
				}
			}
		}

	}

	if(invalidate_next_callback) //Countered!
	{
		invalidate_next_callback  = false;
		if(g.callback_if_countered)
		{
			g.callback_if_countered();
		}
	}
	else
	{
		if(g.callback_if_resolved)
		{
			g.callback_if_resolved();
		}
	}
	stack.pop();
}

