#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <limits>
#include <future>

#include "GameState.hpp"
#include "Aliens/TickTock.hpp" //Alternatively, "Aliens/Aliens.hpp" for all of them

bool is_only_digits(const std::string &s)
{
	        return std::all_of(s.begin(),s.end(),::isdigit);
}

GameState::GameState(unsigned nplayers, CosmicServer &serv) : num_players(nplayers), players(nplayers), destiny_deck(DestinyDeck(nplayers,[this] () { this->update_destiny_discard(); })), invalidate_next_callback(false), player_to_be_plagued(max_player_sentinel), is_second_encounter_for_offense(false), encounter_num(0), server(serv), warp([this] () { this->update_warp(); }), hyperspace_gate([this] () { this->update_hyperspace_gate(); }), defensive_ally_ships([this] () { this->update_defensive_ally_ships(); }), assignments([this] () { this->update_offense(); }, [this] () { this->update_defense(); }), cosmic_discard([this] () { this->update_cosmic_discard(); })
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

const std::string GameState::get_PlanetInfo(const PlanetInfoVector<PlayerColors> &source, const std::string name) const
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

void GameState::update_defense() const
{
	std::string msg("[defense_update] ");
	msg.append(to_string(assignments.get_defense()));
	server.broadcast_message(msg);
}

void GameState::update_cosmic_discard() const
{
	std::string msg("[cosmic_discard_update]\n");
	msg.append(get_cosmic_discard());
	server.broadcast_message(msg);
}

void GameState::update_destiny_discard() const
{
	std::string msg("[destiny_discard_update]\n");
	msg.append(get_destiny_discard());
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
			std::string msg("[alien_update]");
			msg.append(player.get_alien_desc());
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
	GameEvent g(PlayerColors::Invalid,GameEventType::CosmicDeckShuffle);
	resolve_game_event(g);
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
	for(auto i=cosmic_discard.cbegin(),e=cosmic_discard.cend();i!=e;++i)
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

void GameState::broadcast_player_hand_size(const PlayerColors player) const
{
	const unsigned player_hand_size = get_player_const(player).hand_size();
	std::stringstream message;
	message << "[player_hand_size] ";
	message << to_string(player) << ": " << player_hand_size;
	server.broadcast_message(message.str());
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

void GameState::cast_plague(GameEvent &g)
{
	const PlayerColors &casting_player = g.player;
	std::vector<std::string> options;
	for(unsigned i=0; i<players.size(); i++)
	{
		options.push_back(to_string(players[i].color));
	}

	std::string prompt("Which player would you like to harm?\n");
	unsigned chosen_option = prompt_player(casting_player,prompt,options);

	assert(player_to_be_plagued == max_player_sentinel);
	player_to_be_plagued = chosen_option;

	std::stringstream aux_msg;
	aux_msg << "(Targeting the " << to_string(players[player_to_be_plagued].color) << " player)";
	g.aux = aux_msg.str();
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

//NOTE:  
//	 Consider the following scenario:
//	 Blue player is offense during regroup and choose not to cast Mobius Tubes
//	 The Green player, next to act, casts Plague on the Blue Player
//	 The Blue player has another artifact card to discard and chooses to hang on to Mobius Tubes
//	 Once plague has resolved, shouldn't the Blue player be able to play Mobius Tubes? It's still the regroup phase!
//	 More generally, if anything resolves the first time this function is called we need to call it again (what's tricky here is that we also have to be sure that we don't allow events to resolve twice, such as Alien powers...we may have to mark them as 'used')
//TODO: Prompt players even when they have no options here? Slows down the game a bit but does a better job of keeping players aware of what's going on
//	Additionally, the "None" option should probably read 'Proceed to the <next_phase> Phase' (or 'Proceed to the next encounter' for resolution)
void GameState::check_for_game_events_helper(std::set<PlayerColors> &used_aliens_this_phase)
{
	std::vector<PlayerColors> player_order = get_player_order();

	unsigned satisfied_players = 0; //When all of the players decline to make a play (or have no play to make), we can move on
	unsigned current_player_index = 0;
	while(satisfied_players < players.size())
	{
		PlayerInfo &current_player = get_player(player_order[current_player_index]);
		//Starting with the offense, check for valid plays (artifact cards or alien powers) based on the current TurnPhase
		std::vector<GameEvent> valid_plays;
		for(auto i=current_player.hand_begin(),e=current_player.hand_end(); i!=e; ++i)
		{
			if(can_play_card_with_empty_stack(state,*i,current_player.current_role,current_player.alien->get_name()))
			{
				GameEvent g(current_player.color,to_game_event_type(*i));
				valid_plays.push_back(g);
			}
		}

		bool alien_power_available = false;
		bool alien_power_mandatory = false;
		GameEvent alien_power = current_player.can_use_alien_with_empty_stack(state);
		if(alien_power.event_type != GameEventType::None && used_aliens_this_phase.find(current_player.color) == used_aliens_this_phase.end())
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
				satisfied_players = 0;
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
					used_aliens_this_phase.insert(current_player.color);
				}

				if(g.callback_if_action_taken)
					g.callback_if_action_taken();
				g.callback_if_action_taken = nullptr;
				resolve_game_event(g);

				//Recalculate the set of valid plays since either the current play or a response to it could have changed it
				valid_plays.clear();
				for(auto i=current_player.hand_begin(),e=current_player.hand_end(); i!=e; ++i)
				{
					if(can_play_card_with_empty_stack(state,*i,current_player.current_role,current_player.alien->get_name()))
					{
						GameEvent g(current_player.color,to_game_event_type(*i));
						valid_plays.push_back(g);
					}
				}

				if(used_aliens_this_phase.find(current_player.color) == used_aliens_this_phase.end())
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
				if(alien_power_available && alien_power_mandatory && used_aliens_this_phase.find(current_player.color) == used_aliens_this_phase.end())
				{
					server.send_message_to_client(current_player.color,"Mandatory alien power not yet chosen, try again.\n");
				}
				else
				{
					break;
				}
			}
		}
		satisfied_players++;
		current_player_index++;
		if(current_player_index == players.size())
		{
			current_player_index = 0;
		}
	}

	//Recalculate player scores
	update_player_scores();
}

void GameState::check_for_game_events()
{
	//Set of player colors that have had their alien power used this phase of this encounter
	//This prevents alien powers from being used more than once per encounter phase (is this too strict?)
	std::set<PlayerColors> used_aliens_this_phase;

	check_for_game_events_helper(used_aliens_this_phase);
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

        std::stringstream msg;
        msg << "[player_score_update]\n";
        for(auto i=players.begin(),e=players.end();i!=e;++i)
        {
                msg << to_string(i->color) << ": " << i->score << "\n";
        }
	server.broadcast_message(msg.str());

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
			g.callback_if_action_taken = [this,&g] () { this->cast_plague(g); };
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
			g.callback_if_action_taken = [this,&g] { this->setup_reinforcements(g); };
			g.callback_if_resolved = [this,&g] () { this->add_reinforcements(g,2); }; //This lambda needs to capture a reference to g because we want it to have access to the object altered by setup_reinforcements, which will occur later
		break;

		case CosmicCardType::Reinforcement3:
			g.callback_if_action_taken = [this,&g] { this->setup_reinforcements(g); };
			g.callback_if_resolved = [this,&g] () { this->add_reinforcements(g,3); };
		break;

		case CosmicCardType::Reinforcement5:
			g.callback_if_action_taken = [this,&g] { this->setup_reinforcements(g); };
			g.callback_if_resolved = [this,&g] () { this->add_reinforcements(g,5); };
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
				//One of the offense's planets has no ships on it and can immediately be taken over
				if(offense.planets.planet_size(i) == 0)
				{
					valid_home_system_encounters[std::make_pair(assignments.get_offense(),i)] = 0;
				}
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

			//Allow the offense to instantly retake one of their empty planets
			//What if the offense wants to reclaim one on their planets and that planet happens to be empty? From the rules:
			//"When drawing his or her own color, if a player has a home planet with no ships on it at all (enemy or otherwise), then he or she may aim the hyperspace gate at that planet to automatically re-establish a colony there with up to four ships from other colonies. Doing so counts as a successful encounter."
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
				//FIXME: Reorganize this logic so that if the offense chooses to have a home system encounter they choose which one during the Launch phase
				//	 Note that reestablishing a colony on an empty planet as the offense (if possible and chosen) should occur during destiny so there's no change needed there
				for(auto i=valid_home_system_encounters.begin(),e=valid_home_system_encounters.end();i!=e;++i)
				{
					std::stringstream opt;
					if(i->first.first == assignments.get_offense())
					{
						opt << "Immediately reestablish a colony on " << to_string(off) << " Planet " << i->first.second << " using ships from your existing colonies";
					}
					else
					{
						opt << "Have an encounter with the " << to_string(i->first.first) << " player on " << to_string(off) << " Planet " << i->first.second << " (the " << to_string(i->first.first) << " player has " << i->second << " ships on this planet.)";
					}
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
					bool reestablishing = false;
					for(auto i=valid_home_system_encounters.begin(),e=valid_home_system_encounters.end();i!=e;++i)
					{
						if(option == chosen_option)
						{
							home_system_encounter = i->first;
							if(i->first.first == assignments.get_offense())
							{
								reestablishing = true;
							}
							break;
						}
						option++;
					}
					assignments.planet_location = off;
					assignments.planet_id = home_system_encounter.second;

					if(reestablishing)
					{
						//Get valid colonies and send in ships
						//Then skip the rest of the turn and mark this as a successful encounter
						send_in_ships(assignments.get_offense(),true,home_system_encounter);
						assignments.successful_encounter = true;
						assignments.reestablished_colony = true;
					}
					else
					{
						assignments.set_defense(home_system_encounter.first);
						PlayerInfo &def = get_player(assignments.get_defense());
						def.current_role = EncounterRole::Defense;
					}
				}
			}
		}
		else
		{
			assignments.set_defense(drawn);
			assignments.planet_location = drawn;
			PlayerInfo &def = get_player(assignments.get_defense());
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

			assignments.set_defense(players[defense_index].color);
			assignments.planet_location = players[defense_index].color;
			PlayerInfo &def = get_player(assignments.get_defense());
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

			assignments.set_defense(players[defense_index].color);
			assignments.planet_location = players[defense_index].color;
			PlayerInfo &def = get_player(assignments.get_defense());
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

			assignments.set_defense(players[defense_index].color);
			assignments.planet_location = players[defense_index].color;
			PlayerInfo &def = get_player(assignments.get_defense());
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
						assignments.set_defense(players[i].color);
						assignments.planet_location = players[i].color;
						break;
					}
					valid_option++;
				}
			}

			assert(assignments.get_defense() != PlayerColors::Invalid && "Failed to set defensive player after drawing wild destiny card");
			assert(assignments.planet_location != PlayerColors::Invalid && "Failed to set defensive planet location after drawing wild destiny card");

			PlayerInfo &def = get_player(assignments.get_defense());
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
	std::stringstream announce;

	//If the offense is having an encounter on their home system they've already chosen a planet
	if(assignments.planet_location != assignments.get_offense())
	{
		announce << "The offense (" << to_string(assignments.get_offense()) << ") has chosen to have an encounter with the " << to_string(assignments.get_defense()) << " player.\n";
		server.broadcast_message(announce.str());

		std::vector<std::string> options;
		const PlayerInfo &host = get_player(assignments.planet_location);
		for(unsigned i=0; i<host.planets.size(); i++)
		{
			std::stringstream opt;
			opt << to_string(host.color) << " Planet " << i;
			options.push_back(opt.str());
		}
		std::stringstream prompt;
		prompt << "[planet_response] " << to_string(assignments.planet_location) << "\n";
		unsigned chosen_option = prompt_player(assignments.get_offense(),prompt.str(),options);
		assignments.planet_id = chosen_option;
	}

	announce.str("");
	announce << "[targeted_planet]\n";
	announce << "The offense (" << to_string(assignments.get_offense()) << ") will have an encounter with the " << to_string(assignments.get_defense()) << " player on " << to_string(assignments.planet_location) << " Planet " << assignments.planet_id << "\n";
	server.broadcast_message(announce.str());
}

//The latter two arguments here are only used for when the offense can immediately reestablish a colony on one of their own planets
void GameState::send_in_ships(const PlayerColors player, bool custom_destination, const std::pair<PlayerColors,unsigned> dest_planet)
{
	if(custom_destination)
	{
		assert(player == assignments.get_offense() && "send_in_ships with custom_destination only expected to be used for the offense!");
		assert(dest_planet.first == player && "The offense can only reestablish a colony on one of its own planets");
	}
	//List the player's valid colonies and have them choose a colony or none until they've chosen none or they've chosen four
	unsigned launched_ships = 0;
	std::vector< std::pair<PlayerColors,unsigned> > valid_colonies;
	std::pair<PlayerColors,unsigned> chosen_colony;
	do
	{
		valid_colonies = get_valid_colonies(player);
		chosen_colony = prompt_valid_colonies(player,valid_colonies,launched_ships != 0);
		if(chosen_colony.first != PlayerColors::Invalid)
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
				if(custom_destination)
				{
					PlayerInfo &off = get_player(dest_planet.first);
					const unsigned planet_id = dest_planet.second;
					off.planets.planet_push_back(planet_id,dest_planet.first);
				}
				else
				{
					hyperspace_gate.push_back(player); //Add the ship to the hyperspace gate
				}
			}
			//Remove the ship from the chosen colony
			PlayerInfo &host = get_player(chosen_colony.first);
			const unsigned planet_id = chosen_colony.second;
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
	} while(((chosen_colony.first != PlayerColors::Invalid) && launched_ships < 4));
}

std::set<PlayerColors> GameState::invite_allies(const std::set<PlayerColors> &potential_allies, bool offense)
{
	std::set<PlayerColors> invited;
	for(auto i=potential_allies.begin(),e=potential_allies.end();i!=e;++i)
	{
		std::stringstream prompt;
		prompt << "Would you like to invite the " << to_string(*i) << " player as an ally?\n";
		std::vector<std::string> options;
		options.push_back("Y");
		options.push_back("N");
		PlayerColors inviter_color = offense ? assignments.get_offense() : assignments.get_defense();
		unsigned response = prompt_player(inviter_color,prompt.str(),options);

		if(response == 0)
		{
			invited.insert(*i);
		}
	}

	return invited;
}

//Ashwath points out that an expansion has rules that make the acceptances of alliance offers unknown until all players have chosen to avoid biases.
//The original rule book has pretty explicit rules on turn order here so I'm sticking with those rules for now:
//"Only after a player has allied with a side (or declined all invitations) and committed ships does the next player accept or decline an invitation"
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

		if(current_player_color == assignments.get_offense() || current_player_color == assignments.get_defense() || get_valid_colonies(current_player_color).empty())
		{
			//The potential ally is already involved or has no ships to commit to the encounter and therefore cannot be an ally
			continue;
		}

		bool this_player_invited_by_offense = (invited_by_offense.find(current_player_color) != invited_by_offense.end());
		bool this_player_invited_by_defense = (invited_by_defense.find(current_player_color) != invited_by_defense.end());
		if(!this_player_invited_by_offense && !this_player_invited_by_defense)
		{
			continue;
		}
		else
		{
			std::stringstream prompt;
			prompt << "Would you like to form an alliance?\n";
			std::vector<std::string> options;
			if(this_player_invited_by_offense)
			{
				std::stringstream msg;
				msg << "Form alliance with the offense (" << to_string(assignments.get_offense()) << " player)";
				options.push_back(msg.str());
			}
			if(this_player_invited_by_defense)
			{
				std::stringstream msg;
				msg << "Form alliance with the defense (" << to_string(assignments.get_defense()) << " player)";
				options.push_back(msg.str());
			}
			options.push_back("Sit out of this encounter");

			unsigned response = prompt_player(current_player_color,prompt.str(),options);

			if(response != (options.size()-1)) //If the player chose an alliance
			{
				bool allied_with_offense = (this_player_invited_by_offense && response == 0);
				if(allied_with_offense)
				{
					assignments.offensive_allies.insert(std::make_pair(current_player_color,0));
					PlayerInfo &ally = get_player(current_player_color);
					ally.current_role = EncounterRole::OffensiveAlly;
				}
				else //Allied with the defense
				{
					assignments.defensive_allies.insert(std::make_pair(current_player_color,0));
					PlayerInfo &ally = get_player(current_player_color);
					ally.current_role = EncounterRole::DefensiveAlly;
				}
				send_in_ships(current_player_color);
			}
		}
	}
}

void GameState::force_negotiation()
{
	assignments.offensive_encounter_card = CosmicCardType::Negotiate;
	assignments.defensive_encounter_card = CosmicCardType::Negotiate;
	assignments.negotiating = true;

	//If the human is zapped and then someone tries to play emotion control then there shouldn't actually be a deal
	if(!assignments.human_wins_encounter)
		setup_negotiation();
}

void GameState::setup_negotiation()
{
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
	//NOTE: For now we're not adding up scores and sending them to the GUIs like we do for normal attacks and compensation scenarios
	PlayerInfo &offense = get_player(assignments.get_offense());
	for(auto i=hyperspace_gate.begin(),e=hyperspace_gate.end();i!=e;++i)
	{
		if(*i == offense.color)
		{
			move_ship_to_colony(offense,hyperspace_gate);
		}
	}

	std::stringstream prompt;
	prompt << "[needs_response]\n";
	prompt << "[deal_setup]\n";
	prompt << "Offense = " << to_string(assignments.get_offense()) << "\n";
	prompt << "Defense = " << to_string(assignments.get_defense()) << "\n";
	std::vector< std::pair<PlayerColors,unsigned> > offense_valid_colonies = get_valid_colonies(assignments.get_offense()); //A list of planet colors and indices
	std::vector< std::pair<PlayerColors,unsigned> > defense_valid_colonies = get_valid_colonies(assignments.get_defense()); //A list of planet colors and indices
	prompt << "Valid offense colonies:\n";
	for(auto i=offense_valid_colonies.begin(),e=offense_valid_colonies.end();i!=e;++i)
	{
		//Validate that the defense does not already have a colony on this planet
		PlayerInfo &host = get_player(i->first);
		bool defense_found = false;
		for(auto ii=host.planets.planet_cbegin(i->second),ee=host.planets.planet_cend(i->second); ii!=ee; ++ii) //TODO: If iterators are weird here we can use host.planet_size(i->second)
		{
			if(*ii == assignments.get_defense())
			{
				defense_found = true;
				break;
			}
		}
		if(!defense_found)
		{
			prompt << to_string(i->first) << " Planet " << i->second << "\n";
		}
	}
	prompt << "Valid defense colonies:\n";
	for(auto i=defense_valid_colonies.begin(),e=defense_valid_colonies.end();i!=e;++i)
	{
		//Validate that the offense does not already have a colony on this planet
		PlayerInfo &host = get_player(i->first);
		bool offense_found = false;
		for(auto ii=host.planets.planet_cbegin(i->second),ee=host.planets.planet_cend(i->second); ii!=ee; ++ii) //TODO: If iterators are weird here we can use host.planet_size(i->second)
		{
			if(*ii == assignments.get_offense())
			{
				offense_found = true;
				break;
			}
		}
		if(!offense_found)
		{
			prompt << to_string(i->first) << " Planet " << i->second << "\n";
		}
	}

	server.send_message_to_client(assignments.get_offense(),prompt.str());
	server.send_message_to_client(assignments.get_defense(),prompt.str());

	std::future<std::string> offense_proposal = std::async(std::launch::async,&CosmicServer::receive_message_from_client,this->server,assignments.get_offense());
	std::future<std::string> defense_proposal = std::async(std::launch::async,&CosmicServer::receive_message_from_client,this->server,assignments.get_defense());

	//Check for a response from either player
	//TODO: Add a 1-2 minute timer that causes the deal to fail if time runs out
	//TODO: Support trading cards (either at random or by choice and eventually allow the declaration of specific cards or even a specific type of card: "highest attack card" or  "any artifact card")
	bool offense_done = false;
	bool defense_done = false;
	std::chrono::milliseconds span(500); //How long to wait for each client during each wait_for call

	std::pair<PlayerColors,unsigned> offense_proposed_colony_to_offense;
	std::pair<PlayerColors,unsigned> offense_proposed_colony_to_defense;
	unsigned offense_proposed_cards_to_offense = 0;
	bool offense_proposed_cards_to_offense_random = false;
	unsigned offense_proposed_cards_to_defense = 0;
	bool offense_proposed_cards_to_defense_random = false;

	std::pair<PlayerColors,unsigned> defense_proposed_colony_to_offense;
	std::pair<PlayerColors,unsigned> defense_proposed_colony_to_defense;
	unsigned defense_proposed_cards_to_offense = 0;
	bool defense_proposed_cards_to_offense_random = false;
	unsigned defense_proposed_cards_to_defense = 0;
	bool defense_proposed_cards_to_defense_random = false;

	while(1)
	{
		if(!offense_done && offense_proposal.wait_for(span) == std::future_status::ready)
		{
			std::string offense_msg = offense_proposal.get();
			if(offense_msg.find("[propose_deal]") != std::string::npos)
			{
				//The offense has proposed a deal, record it and send it to the defense
				const std::string offense_receives_colony_delim("Offense receives colony: ");
				auto defense_to_offense_colony_loc = offense_msg.find(offense_receives_colony_delim) + offense_receives_colony_delim.size();
				assert(defense_to_offense_colony_loc != std::string::npos && "Failed to parse offensive deal proposal!");
				auto defense_to_offense_colony_endloc = offense_msg.find('\n',defense_to_offense_colony_loc);
				const std::string colony_to_offense(offense_msg.begin()+defense_to_offense_colony_loc,offense_msg.begin()+defense_to_offense_colony_endloc); //In the form of "Purple Planet 4" or "None"
				const std::string planet_txt_delim(" Planet ");

				if(colony_to_offense.compare("None") == 0)
				{
					offense_proposed_colony_to_offense = std::make_pair(PlayerColors::Invalid,0);
				}
				else
				{
					auto planet_txt_loc = colony_to_offense.find(planet_txt_delim) + planet_txt_delim.size();
					assert(planet_txt_loc != std::string::npos && "Failed to parse offensive deal proposal!");
					const std::string colony_to_offense_planet_color(colony_to_offense.begin(),colony_to_offense.begin()+colony_to_offense.find(planet_txt_delim));
					const std::string colony_to_offense_planet_num_str(colony_to_offense.begin()+planet_txt_loc,colony_to_offense.end());
					std::cout << "Offense proposed colony to offense: " << colony_to_offense_planet_color << " Planet " << colony_to_offense_planet_num_str << "\n";
					offense_proposed_colony_to_offense = std::make_pair(to_color(colony_to_offense_planet_color),std::stoi(colony_to_offense_planet_num_str));
				}

				const std::string defense_receives_colony_delim("Defense receives colony: ");
				auto offense_to_defense_colony_loc = offense_msg.find(defense_receives_colony_delim) + defense_receives_colony_delim.size();
				assert(offense_to_defense_colony_loc != std::string::npos && "Failed to parse offensive deal proposal!");
				auto offense_to_defense_colony_endloc = offense_msg.find('\n',offense_to_defense_colony_loc);
				const std::string colony_to_defense(offense_msg.begin()+offense_to_defense_colony_loc,offense_msg.begin()+offense_to_defense_colony_endloc); //In the form of "Purple Planet 4"
				if(colony_to_defense.compare("None") == 0)
				{
					offense_proposed_colony_to_defense = std::make_pair(PlayerColors::Invalid,0);
				}
				else
				{
					auto planet_txt_loc = colony_to_defense.find(planet_txt_delim) + planet_txt_delim.size();
					assert(planet_txt_loc != std::string::npos && "Failed to parse offensive deal proposal!");
					const std::string colony_to_defense_planet_color(colony_to_defense.begin(),colony_to_defense.begin()+colony_to_defense.find(planet_txt_delim));
					const std::string colony_to_defense_planet_num_str(colony_to_defense.begin()+planet_txt_loc,colony_to_defense.end());
					std::cout << "Offense proposed colony to defense: " << colony_to_defense_planet_color << " Planet " << colony_to_defense_planet_num_str << "\n";
					offense_proposed_colony_to_defense = std::make_pair(to_color(colony_to_defense_planet_color),std::stoi(colony_to_defense_planet_num_str));
				}

				const std::string cards_to_offense("Number of cards for the offense to receive from the defense: ");
				auto cards_to_offense_loc = offense_msg.find(cards_to_offense);
				assert(cards_to_offense_loc != std::string::npos && "Failed to parse cards from offensive deal proposal!");
				auto cards_to_offense_type_loc = offense_msg.find(" (", cards_to_offense_loc);
				assert(cards_to_offense_type_loc != std::string::npos && "Failed to parse cards from offensive deal proposal!");
				const std::string cards_to_offense_num(offense_msg.begin()+cards_to_offense_loc+cards_to_offense.size(),offense_msg.begin()+cards_to_offense_type_loc);
				offense_proposed_cards_to_offense = std::stoi(cards_to_offense_num);
				auto cards_to_offense_endline = offense_msg.find(")\n", cards_to_offense_loc);
				assert(cards_to_offense_endline != std::string::npos && "Failed to parse cards from offensive deal proposal!");
				const std::string cards_to_offense_type(offense_msg.begin()+cards_to_offense_type_loc+2,offense_msg.begin()+cards_to_offense_endline);
				if(cards_to_offense_type.compare("at random") == 0)
				{
					offense_proposed_cards_to_offense_random = true;
				}
				else if(cards_to_offense_type.compare("by choice") == 0)
				{
					offense_proposed_cards_to_offense_random = false;
				}
				else
				{
					std::cerr << "cards_to_offense_type: " << cards_to_offense_type << "\n";
					assert(0 && "Invalid value of cards_to_offense_type");
				}

				const std::string cards_to_defense("Number of cards for the defense to receive from the offense: ");
				auto cards_to_defense_loc = offense_msg.find(cards_to_defense);
				assert(cards_to_defense_loc != std::string::npos && "Failed to parse cards from offensive deal proposal!");
				auto cards_to_defense_type_loc = offense_msg.find(" (", cards_to_defense_loc);
				assert(cards_to_defense_type_loc != std::string::npos && "Failed to parse cards from offensive deal proposal");
				const std::string cards_to_defense_num(offense_msg.begin()+cards_to_defense_loc+cards_to_defense.size(),offense_msg.begin()+cards_to_defense_type_loc);
				offense_proposed_cards_to_defense = std::stoi(cards_to_defense_num);
				auto cards_to_defense_endline = offense_msg.find(")\n", cards_to_defense_loc);
				assert(cards_to_defense_endline != std::string::npos && "Failed to parse cards from offensive deal proposal!");
				const std::string cards_to_defense_type(offense_msg.begin()+cards_to_defense_type_loc+2,offense_msg.begin()+cards_to_defense_endline);
				if(cards_to_defense_type.compare("at random") == 0)
				{
					offense_proposed_cards_to_defense_random = true;
				}
				else if(cards_to_defense_type.compare("by choice") == 0)
				{
					offense_proposed_cards_to_defense_random = false;
				}
				else
				{
					std::cerr << "cards_to_defense_type: " << cards_to_defense_type << "\n";
					assert(0 && "Invalid value of cards_to_defense_type");
				}

				server.send_message_to_client(assignments.get_defense(),offense_msg);
				offense_proposal = std::async(std::launch::async,&CosmicServer::receive_message_from_client,this->server,assignments.get_offense()); //Wait for the next message
			}
			else if(offense_msg.find("[reject_deal]") != std::string::npos)
			{
				//We rejected a deal from the defense; inform the defense that they can attempt to propose another deal if they are so inclined
				server.send_message_to_client(assignments.get_defense(),offense_msg);
				offense_proposal = std::async(std::launch::async,&CosmicServer::receive_message_from_client,this->server,assignments.get_offense()); //Wait for the next message
			}
			else if(offense_msg.find("[accept_deal]") != std::string::npos)
			{
				std::cout << "The offense has accepted a deal from the defense\n";
				deal_params.successful = true;
				deal_params.num_cards_to_offense = defense_proposed_cards_to_offense;
				deal_params.cards_to_offense_chosen_randomly = defense_proposed_cards_to_offense_random;
				deal_params.num_cards_to_defense = defense_proposed_cards_to_defense;
				deal_params.cards_to_defense_chosen_randomly = defense_proposed_cards_to_defense_random;

				if(offense_msg.find("offense will establish a colony") != std::string::npos)
				{
					deal_params.offense_receives_colony = true;
					deal_params.colony_for_offense = defense_proposed_colony_to_offense; //Use the defense proposal here, because that's what was accepted
				}
				else
				{
					deal_params.offense_receives_colony = false;
				}

				if(offense_msg.find("defense will establish a colony") != std::string::npos)
				{
					deal_params.defense_receives_colony = true;
					deal_params.colony_for_defense = defense_proposed_colony_to_defense; //Use the defense proposal here, because that's what was accepted
				}
				else
				{
					deal_params.defense_receives_colony = false;
				}

				//We need the defense_proposal thread to return before this parent function exits; send a message to the other player notifying them that the deal was accepted and they can send an ack that we can wait on for a more graceful exit here
				server.send_message_to_client(assignments.get_defense(),offense_msg);
				offense_done = true;
			}
			else if(offense_msg.find("[ack]") != std::string::npos)
			{
				break;
			}
			else
			{
				std::cerr << "Error: Unexpected message from client: " << offense_msg << "\n";
				assert(0 && "Unexpected message from client");
			}
		}

		if(!defense_done && defense_proposal.wait_for(span) == std::future_status::ready)
		{
			std::string defense_msg = defense_proposal.get();
			if(defense_msg.find("[propose_deal]") != std::string::npos)
			{
				//The defense has proposed a deal, record it and send it to the offense
				const std::string offense_receives_colony_delim("Offense receives colony: ");
				auto defense_to_offense_colony_loc = defense_msg.find(offense_receives_colony_delim) + offense_receives_colony_delim.size();
				assert(defense_to_offense_colony_loc != std::string::npos && "Failed to parse offensive deal proposal!");
				auto defense_to_offense_colony_endloc = defense_msg.find('\n',defense_to_offense_colony_loc);
				const std::string colony_to_offense(defense_msg.begin()+defense_to_offense_colony_loc,defense_msg.begin()+defense_to_offense_colony_endloc); //In the form of "Purple Planet 4" or "None"
				const std::string planet_txt_delim(" Planet ");

				if(colony_to_offense.compare("None") == 0)
				{
					defense_proposed_colony_to_offense = std::make_pair(PlayerColors::Invalid,0);
				}
				else
				{
					auto planet_txt_loc = colony_to_offense.find(planet_txt_delim) + planet_txt_delim.size();
					assert(planet_txt_loc != std::string::npos && "Failed to parse offensive deal proposal!");
					const std::string colony_to_offense_planet_color(colony_to_offense.begin(),colony_to_offense.begin()+colony_to_offense.find(planet_txt_delim));
					const std::string colony_to_offense_planet_num_str(colony_to_offense.begin()+planet_txt_loc,colony_to_offense.end());
					std::cout << "Defense proposed colony to offense: " << colony_to_offense_planet_color << " Planet " << colony_to_offense_planet_num_str << "\n";
					defense_proposed_colony_to_offense = std::make_pair(to_color(colony_to_offense_planet_color),std::stoi(colony_to_offense_planet_num_str));
				}

				const std::string defense_receives_colony_delim("Defense receives colony: ");
				auto offense_to_defense_colony_loc = defense_msg.find(defense_receives_colony_delim) + defense_receives_colony_delim.size();
				assert(offense_to_defense_colony_loc != std::string::npos && "Failed to parse offensive deal proposal!");
				auto offense_to_defense_colony_endloc = defense_msg.find('\n',offense_to_defense_colony_loc);
				const std::string colony_to_defense(defense_msg.begin()+offense_to_defense_colony_loc,defense_msg.begin()+offense_to_defense_colony_endloc); //In the form of "Purple Planet 4"
				if(colony_to_defense.compare("None") == 0)
				{
					defense_proposed_colony_to_defense = std::make_pair(PlayerColors::Invalid,0);
				}
				else
				{
					auto planet_txt_loc = colony_to_defense.find(planet_txt_delim) + planet_txt_delim.size();
					assert(planet_txt_loc != std::string::npos && "Failed to parse offensive deal proposal!");
					const std::string colony_to_defense_planet_color(colony_to_defense.begin(),colony_to_defense.begin()+colony_to_defense.find(planet_txt_delim));
					const std::string colony_to_defense_planet_num_str(colony_to_defense.begin()+planet_txt_loc,colony_to_defense.end());
					std::cout << "Defense proposed colony to defense: " << colony_to_defense_planet_color << " Planet " << colony_to_defense_planet_num_str << "\n";
					defense_proposed_colony_to_defense = std::make_pair(to_color(colony_to_defense_planet_color),std::stoi(colony_to_defense_planet_num_str));
				}

				const std::string cards_to_offense("Number of cards for the offense to receive from the defense: ");
				auto cards_to_offense_loc = defense_msg.find(cards_to_offense);
				assert(cards_to_offense_loc != std::string::npos && "Failed to parse cards from defensive deal proposal!");
				auto cards_to_offense_type_loc = defense_msg.find(" (", cards_to_offense_loc);
				assert(cards_to_offense_type_loc != std::string::npos && "Failed to parse cards from defensive deal proposal!");
				const std::string cards_to_offense_num(defense_msg.begin()+cards_to_offense_loc+cards_to_offense.size(),defense_msg.begin()+cards_to_offense_type_loc);
				defense_proposed_cards_to_offense = std::stoi(cards_to_offense_num);
				auto cards_to_offense_endline = defense_msg.find(")\n", cards_to_offense_loc);
				assert(cards_to_offense_endline != std::string::npos && "Failed to parse cards from defensive deal proposal!");
				const std::string cards_to_offense_type(defense_msg.begin()+cards_to_offense_type_loc+2,defense_msg.begin()+cards_to_offense_endline);
				if(cards_to_offense_type.compare("at random") == 0)
				{
					defense_proposed_cards_to_offense_random = true;
				}
				else if(cards_to_offense_type.compare("by choice") == 0)
				{
					defense_proposed_cards_to_offense_random = false;
				}
				else
				{
					std::cerr << "cards_to_offense_type: " << cards_to_offense_type << "\n";
					assert(0 && "Invalid value of cards_to_offense_type");
				}

				const std::string cards_to_defense("Number of cards for the defense to receive from the offense: ");
				auto cards_to_defense_loc = defense_msg.find(cards_to_defense);
				assert(cards_to_defense_loc != std::string::npos && "Failed to parse cards from defensive deal proposal!");
				auto cards_to_defense_type_loc = defense_msg.find(" (", cards_to_defense_loc);
				assert(cards_to_defense_type_loc != std::string::npos && "Failed to parse cards from defensive deal proposal");
				const std::string cards_to_defense_num(defense_msg.begin()+cards_to_defense_loc+cards_to_defense.size(),defense_msg.begin()+cards_to_defense_type_loc);
				defense_proposed_cards_to_defense = std::stoi(cards_to_defense_num);
				auto cards_to_defense_endline = defense_msg.find(")\n", cards_to_defense_loc);
				assert(cards_to_defense_endline != std::string::npos && "Failed to parse cards from defensive deal proposal!");
				const std::string cards_to_defense_type(defense_msg.begin()+cards_to_defense_type_loc+2,defense_msg.begin()+cards_to_defense_endline);
				if(cards_to_defense_type.compare("at random") == 0)
				{
					defense_proposed_cards_to_defense_random = true;
				}
				else if(cards_to_defense_type.compare("by choice") == 0)
				{
					defense_proposed_cards_to_defense_random = false;
				}
				else
				{
					std::cerr << "cards_to_defense_type: " << cards_to_defense_type << "\n";
					assert(0 && "Invalid value of cards_to_defense_type");
				}

				server.send_message_to_client(assignments.get_offense(),defense_msg);
				defense_proposal = std::async(std::launch::async,&CosmicServer::receive_message_from_client,this->server,assignments.get_defense()); //Wait for the next message
			}
			else if(defense_msg.find("[reject_deal]") != std::string::npos)
			{
				//We rejected a deal from the offense; inform the offense that they can attempt to propose another deal if they are so inclined
				server.send_message_to_client(assignments.get_offense(),defense_msg);
				defense_proposal = std::async(std::launch::async,&CosmicServer::receive_message_from_client,this->server,assignments.get_defense()); //Wait for the next message
			}
			else if(defense_msg.find("[accept_deal]") != std::string::npos)
			{
				std::cout << "The defense has accepted a deal from the offense\n";
				deal_params.successful = true;
				deal_params.num_cards_to_offense = offense_proposed_cards_to_offense;
				deal_params.cards_to_offense_chosen_randomly = offense_proposed_cards_to_offense_random;
				deal_params.num_cards_to_defense = offense_proposed_cards_to_defense;
				deal_params.cards_to_defense_chosen_randomly = offense_proposed_cards_to_defense_random;

				if(defense_msg.find("offense will establish a colony") != std::string::npos)
				{
					deal_params.offense_receives_colony = true;
					deal_params.colony_for_offense = offense_proposed_colony_to_offense; //Use the offense proposal here, because that's what was accepted
				}
				else
				{
					deal_params.offense_receives_colony = false;
				}

				if(defense_msg.find("defense will establish a colony") != std::string::npos)
				{
					deal_params.defense_receives_colony = true;
					deal_params.colony_for_defense = offense_proposed_colony_to_defense; //Use the offense proposal here, because that's what was accepted
				}
				else
				{
					deal_params.defense_receives_colony = false;
				}

				//We need the defense_proposal thread to return before this parent function exits; send a message to the other player notifying them that the deal was accepted and they can send an ack that we can wait on for a more graceful exit here
				server.send_message_to_client(assignments.get_offense(),defense_msg);
				defense_done = true;
			}
			else if(defense_msg.find("[ack]") != std::string::npos)
			{
				break;
			}
			else
			{
				std::cerr << "Error: Unexpected message from client: " << defense_msg << "\n";
				assert(0 && "Unexpected message from client");
			}
		}
	}

	bool valid_deal = (deal_params.offense_receives_colony || deal_params.defense_receives_colony || deal_params.num_cards_to_offense > 0 || deal_params.num_cards_to_defense > 0);
	assert(valid_deal && "The client did not enforce a valid deal!");
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
		std::vector< std::pair<PlayerColors,unsigned> > defense_valid_colonies = get_valid_colonies(assignments.get_defense()); //A list of planet colors and indices
		std::vector< std::pair<PlayerColors,unsigned> > offense_valid_colonies = get_valid_colonies(assignments.get_offense()); //A list of planet colors and indices
		if(deal_params.offense_receives_colony)
		{
			//TODO: Is there any logic from the deal making process to enforce these assertions?
			assert(!offense_valid_colonies.empty());
			assert(!defense_valid_colonies.empty());

			//The offense must choose one of their existing colonies to take a ship from! We already have offense_valid_colonies for this purpose
			std::string msg("Choose one of your colonies to take a ship from to establish the new colony.\n");
			server.send_message_to_client(assignments.get_offense(),msg);
			const std::pair<PlayerColors,unsigned> chosen_home_colony = prompt_valid_colonies(assignments.get_offense(),offense_valid_colonies);

			//Now actually remove a ship from the colony
			PlanetInfo &home_planet = get_player(chosen_home_colony.first).planets.get_planet(chosen_home_colony.second);
			for(auto i=home_planet.begin();i!=home_planet.end();)
			{
				if(*i == assignments.get_offense())
				{
					get_player(chosen_home_colony.first).planets.planet_erase(chosen_home_colony.second,i);
					break;
				}
				else
				{
					++i;
				}
			}

			const std::pair<PlayerColors,unsigned> chosen_colony = deal_params.colony_for_offense;

			//Finally, add the ship to the newly established colony
			PlayerInfo &player_with_chosen_colony = get_player(chosen_colony.first);
			player_with_chosen_colony.planets.planet_push_back(chosen_colony.second,assignments.get_offense());
		}
		if(deal_params.defense_receives_colony)
		{
			//TODO: Is there any logic from the deal making process to enforce these assertions?
			assert(!offense_valid_colonies.empty());
			assert(!defense_valid_colonies.empty());

			//The defense must choose one of their existing colonies to take a ship from! We already have defense_valid_colonies for this purpose
			std::string msg("Choose one of your colonies to take a ship from to establish the new colony.\n");
			server.send_message_to_client(assignments.get_defense(),msg);
			const std::pair<PlayerColors,unsigned> chosen_home_colony = prompt_valid_colonies(assignments.get_defense(),defense_valid_colonies);

			//Now actually remove a ship from the colony
			PlanetInfo &home_planet = get_player(chosen_home_colony.first).planets.get_planet(chosen_home_colony.second);
			for(auto i=home_planet.begin();i!=home_planet.end();)
			{
				if(*i == assignments.get_defense())
				{
					get_player(chosen_home_colony.first).planets.planet_erase(chosen_home_colony.second,i);
					break;
				}
				else
				{
					++i;
				}
			}

			const std::pair<PlayerColors,unsigned> chosen_colony = deal_params.colony_for_defense;

			//Finally, add the ship to the newly established colony
			PlayerInfo &player_with_chosen_colony = get_player(chosen_colony.first);
			player_with_chosen_colony.planets.planet_push_back(chosen_colony.second,assignments.get_defense());
		}
		std::vector<CosmicCardType> cards_to_offense;
		//Choose which cards will be taken from the defense
		PlayerInfo &defense = get_player(assignments.get_defense());
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
				prompt << "Choose one of your cards to give to the offense.\n";
				for(unsigned i=0; i<deal_params.num_cards_to_offense; i++)
				{
					std::vector<std::string> options;
					for(unsigned i=0; i<defense.hand_size(); i++)
					{
						options.push_back(to_string(defense.hand_get(i)));
					}
					unsigned chosen_option = prompt_player(assignments.get_defense(),prompt.str(),options);
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
				prompt << "Choose one of your cards to give to the defense.\n";
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
		lose_ships_to_warp(assignments.get_defense(),3);
	}
}

void GameState::broadcast_encounter_scores() const
{
	std::stringstream announce;
	announce << "[score_update] Total score after adding ships (ties go to the defense): Offense = " << assignments.offense_attack_value << "; Defense = " << assignments.defense_attack_value << "\n";
	server.broadcast_message(announce.str());
}

void GameState::setup_compensation()
{
	//The player with the negotiate loses, but collects compensation later
	if(assignments.offensive_encounter_card == CosmicCardType::Negotiate)
	{
		assignments.player_receiving_compensation = assignments.get_offense();
		assignments.player_giving_compensation = assignments.get_defense();
		assignments.defense_attack_value = static_cast<unsigned>(assignments.defensive_encounter_card); //Defense played an attack card
	}
	else
	{
		assignments.player_receiving_compensation = assignments.get_defense();
		assignments.player_giving_compensation = assignments.get_offense();
		assignments.offense_attack_value = static_cast<unsigned>(assignments.offensive_encounter_card); //Offense played an attack card
	}

	//Update scores just in case that matters for some odd edge case
	add_score_from_ships();
	broadcast_encounter_scores();

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

void GameState::add_score_from_ships()
{
	std::stringstream announce;
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
		if(*i == assignments.get_defense())
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
	server.broadcast_message(announce.str());
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

	add_score_from_ships();
	broadcast_encounter_scores();
}

void GameState::offense_win_resolution()
{
	//Defense and defensive ally ships go to the warp
	PlanetInfo &encounter_planet = get_player(assignments.planet_location).planets.get_planet(assignments.planet_id);
	for(auto i=encounter_planet.begin();i!=encounter_planet.end();)
	{
		if(*i == assignments.get_defense())
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

	//One trigger for each winning playter
	GameEvent g(assignments.get_offense(),GameEventType::EncounterWin);
	resolve_game_event(g);
	for(auto i=assignments.offensive_allies.begin(),e=assignments.offensive_allies.end();i!=e;++i)
	{
		GameEvent g(i->first,GameEventType::EncounterWin);
		resolve_game_event(g);
	}
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

	GameEvent g(assignments.get_defense(),GameEventType::DefensiveEncounterWin);
	resolve_game_event(g);

	GameEvent g2(assignments.get_defense(),GameEventType::EncounterWin);
	resolve_game_event(g2);
	for(auto i=assignments.defensive_allies.begin(),e=assignments.defensive_allies.end();i!=e;++i)
	{
		GameEvent g(i->first,GameEventType::EncounterWin);
		resolve_game_event(g);
	}
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
			if(*i == assignments.get_defense())
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

//Called upon cast of reinforcements (so far these can't be countered but the player should choose which side they're targeting immediately)
void GameState::setup_reinforcements(GameEvent &g)
{
	const PlayerColors player = g.player;

	bool player_is_offense;
	if(player == assignments.get_offense() || assignments.offensive_allies.find(player) != assignments.offensive_allies.end())
	{
		player_is_offense = true;
	}
	else if(player == assignments.get_defense() || assignments.defensive_allies.find(player) != assignments.defensive_allies.end())
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
		g.reinforcements_to_offense = true;
	}
	else
	{
		g.reinforcements_to_offense = false;
	}
}

//Called upon resolution of reinforcements
void GameState::add_reinforcements(const GameEvent &g, const unsigned value, bool can_choose_side)
{
	const PlayerColors player = g.player;

	bool player_is_offense;
	if(player == assignments.get_offense() || assignments.offensive_allies.find(player) != assignments.offensive_allies.end())
	{
		player_is_offense = true;
	}
	else if(player == assignments.get_defense() || assignments.defensive_allies.find(player) != assignments.defensive_allies.end())
	{
		player_is_offense = false;
	}
	else
	{
		assert(0 && "Invalid player casted reinforcement card!");
	}

	std::stringstream announce;
	if(can_choose_side)
	{

		if(g.reinforcements_to_offense)
		{
			assignments.offense_attack_value += value;
		}
		else
		{
			assignments.defense_attack_value += value;
		}

		announce << "[score_update] After reinforcements, the revised score is (ties go to the defense): Offense = " << assignments.offense_attack_value << "; Defense = " << assignments.defense_attack_value << "\n";
	}
	else //Human Alien power: the "reinforcements" must be applied to the side of the Human
	{
		if(player_is_offense)
		{
			assignments.offense_attack_value += value;
		}
		else
		{
			assignments.defense_attack_value += value;
		}

		announce << "[score_update] After applying the Human's alien power, the revised score is (ties go to the defense): Offense = " << assignments.offense_attack_value << "; Defense = " << assignments.defense_attack_value << "\n";
	}

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

void GameState::update_tick_tock_tokens(unsigned tokens) const
{
	std::stringstream message;
	message << "[alien_aux_update]\n";
	message << "Tick-Tock tokens remaining: " << tokens << "\n";
	server.broadcast_message(message.str());

	if(tokens == 0)
	{
		message.str(std::string()); //Clear the stringstream
		message << "[tick_tock_win_condition] Tick-Tock has run out of tokens and has won the game!\n";
		server.broadcast_message(message.str());
		std::exit(0); //TODO: A more graceful cleanup?
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
			std::string prompt("You just had your first successful encounter of the turn. Would you like to have a second encounter?\n");
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

	//The offense reestablished a colony so there's nothing else to do for this encounter
	if(assignments.reestablished_colony)
	{
		end_of_turn_clean_up();
		return;
	}

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
		if((players[i].color != assignments.get_offense()) && (players[i].color != assignments.get_defense()))
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
	PlayerInfo &defense = get_player(assignments.get_defense());
	if(!defense.has_encounter_cards_in_hand())
	{
		discard_and_draw_new_hand(defense);
	}

	//Before cards are selected effects can now resolve
	check_for_game_events();

	std::string prompt("Which encounter card would you like to play?\n");
	std::vector<std::string> options;
	std::vector<CosmicCardType> cards_to_be_discarded;
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
				cards_to_be_discarded.push_back(*i);
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
	response = prompt_player(assignments.get_defense(),prompt,options);

	option = 0;
	for(auto i=defense.hand_begin(),e=defense.hand_end();i!=e;++i)
	{
		if(static_cast<unsigned>(*i) <= static_cast<unsigned>(CosmicCardType::Morph))
		{
			if(option == response)
			{
				assignments.defensive_encounter_card = *i;
				cards_to_be_discarded.push_back(*i);
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

	//Now that the cards are known, we can add them to the discard pile (NOTE: game state effects might dictate that these cards are discarded later, during resolution)
	for(auto i=cards_to_be_discarded.begin(),e=cards_to_be_discarded.end();i!=e;++i)
	{
		cosmic_discard.push_back(*i);
	}
	cards_to_be_discarded.clear();

	//Good primer on negotiation specifics: https://boardgamegeek.com/thread/1212948/question-about-trading-cards-during-negotiate/page/1
	assignments.negotiating = ((assignments.offensive_encounter_card == CosmicCardType::Negotiate && assignments.defensive_encounter_card == CosmicCardType::Negotiate) || (assignments.offensive_encounter_card == CosmicCardType::Negotiate && assignments.defensive_encounter_card == CosmicCardType::Morph) || (assignments.offensive_encounter_card == CosmicCardType::Morph && assignments.defensive_encounter_card == CosmicCardType::Negotiate));
	bool compensating = (!assignments.negotiating && (assignments.offensive_encounter_card == CosmicCardType::Negotiate || assignments.defensive_encounter_card == CosmicCardType::Negotiate));
	if(assignments.negotiating)
	{
		setup_negotiation();
	}
	else if(compensating)
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
	else if(assignments.negotiating)
	{
		resolve_negotiation();
	}
	else if(compensating)
	{
		resolve_compensation();
	}
	else
	{
		resolve_attack();
	}

	end_of_turn_clean_up();
}

void GameState::end_of_turn_clean_up()
{
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
	PlayerInfo &defense = get_player(assignments.get_defense());

	std::vector<CosmicCardType> tmp = offense.get_hand_data();
	offense.set_hand_data(defense.get_hand_data());
	defense.set_hand_data(tmp);
}


//FIXME: Don't ships on the hyperspace gate, defensive allies, etc. count for most (or all) use cases here too?
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

//TODO: Add a prompt message here that gets passed onto the GUI so the player has a better understanding of what they're choosing a colony for
const std::pair<PlayerColors,unsigned> GameState::prompt_valid_colonies(const PlayerColors color, const std::vector< std::pair<PlayerColors,unsigned> > &valid_colonies, bool allow_none)
{
	std::stringstream prompt;
	prompt << "[colony_response] The " << to_string(color) << " player has the following valid colonies to choose from:\n";
	std::vector<std::string> options;
	for(unsigned i=0; i<valid_colonies.size(); i++)
	{
		std::stringstream opt;
		opt << to_string(valid_colonies[i].first) << " Planet " << valid_colonies[i].second;
		options.push_back(opt.str());
	}
	if(allow_none)
	{
		options.push_back("None");
	}
	unsigned chosen_option = prompt_player(color,prompt.str(),options);

	if(chosen_option == valid_colonies.size()) //The player chose the "none" option
	{
		return std::make_pair(PlayerColors::Invalid,0);
	}
	else
	{
		return valid_colonies[chosen_option];
	}
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
void GameState::move_ship_to_colony(PlayerInfo &p, PlanetInfoVector<PlayerColors> &source)
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
		bool placed_on_hyperspace_gate = false;
		if(p.color == assignments.get_offense())
		{
			unsigned num_offensive_ships_in_hyperspace_gate = 0;
			for(auto i=hyperspace_gate.cbegin(),e=hyperspace_gate.cend();i!=e;++i)
			{
				if(*i == p.color)
					num_offensive_ships_in_hyperspace_gate++;
			}

			if(num_offensive_ships_in_hyperspace_gate < 4)
			{
				std::stringstream announce;
				announce << "The  " << to_string(p.color) << " player has no valid colonies! Placing the ship directly on the hyperspace gate.\n";
				server.broadcast_message(announce.str());
				hyperspace_gate.push_back(p.color);
				placed_on_hyperspace_gate = true;
			}
		}

		if(!placed_on_hyperspace_gate)
		{
			//No colony to return the ship to!
			//"If a situation arises where a player must relocate ships but he or she has no colonies anywhere on the board, those ships go to the warp." (https://boardgames.stackexchange.com/questions/13245/what-happens-when-a-player-loses-all-of-their-colonies-in-cosmic-encounter)
			std::stringstream announce;
			if(p.color == assignments.get_offense())
			{
				announce << "The " << to_string(p.color) << " player already has four ships on the hyperspace gate. Since they have no valid colonies, additional recovered ships go directly to the warp.\n";
			}
			else
			{
				announce << "The " << to_string(p.color) << " player has no valid colonies! Their retrieved ship goes directly to the warp.\n";
			}
			server.broadcast_message(announce.str());
			warp.push_back(std::make_pair(p.color,encounter_num));
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
			//No colony to return the ship to! The ship stays in the warp
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
	for(auto i=cosmic_discard.cbegin(),e=cosmic_discard.cend();i!=e;++i)
	{
		if(i != cosmic_discard.cbegin())
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

void GameState::establish_colony_on_opponent_planet(const PlayerColors c)
{
	//Form a list of opponent planets where the player establishing the colony does not already have a colony
	std::vector<std::string> options;
	std::vector< std::pair<PlayerColors,unsigned> > option_values;
	for(unsigned i=0; i<players.size(); i++)
	{
		if(players[i].color == c)
		{
			continue;
		}
		for(unsigned planet=0; planet<players[i].planets.size(); planet++)
		{
			bool establishing_player_has_ship = false;
			for(unsigned ship=0; ship<players[i].planets.planet_size(planet); ship++)
			{
				if(players[i].planets.get_ship(planet,ship) == c)
				{
					establishing_player_has_ship = true;
					break;
				}
			}

			if(!establishing_player_has_ship)
			{
				std::stringstream opt;
				opt << to_string(players[i].color) << " Planet " << planet;
				options.push_back(opt.str());
				option_values.push_back(std::make_pair(players[i].color,planet));
			}
		}
	}
	options.push_back("None"); //Technically the player can choose not to establish a colony. Presumably they wouldn't play the flare then, but hey, it's a legal option

	std::stringstream prompt;
	prompt << "[planet_response]\n";
	unsigned chosen_option = prompt_player(c,prompt.str(),options);

	if(chosen_option == (options.size()-1))
	{
		//The player chose to do nothing
		return;
	}

	const std::pair<PlayerColors,unsigned> new_colony = option_values[chosen_option];
	for(unsigned ship=0; ship<4; ship++)
	{
		//Have the player choose up to 4 ships from their valid colonies to establish the new colony
		std::vector< std::pair<PlayerColors,unsigned> > valid_colonies = get_valid_colonies(c); //Update this each loop in case the player decides to take their last ship from a different colony
		if(!valid_colonies.size())
		{
			return;
		}

		//Don't include the newly established colony as one of the valid colonies
		for(auto i=valid_colonies.begin(),e=valid_colonies.end();i!=e;++i)
		{
			if(i->first == new_colony.first && i->second == new_colony.second)
			{
				valid_colonies.erase(i);
				break;
			}
		}

		bool allow_none = (ship!=0);
		const std::pair<PlayerColors,unsigned> old_colony = prompt_valid_colonies(c,valid_colonies,allow_none);

		if(allow_none && old_colony.first == PlayerColors::Invalid) //The player chose none
		{
			break;
		}

		//Move the ship from the old colony to the new one
		get_player(new_colony.first).planets.planet_push_back(new_colony.second,c);
		PlayerInfo &old_host = get_player(old_colony.first);
		for(auto i=old_host.planets.planet_begin(old_colony.second),e=old_host.planets.planet_end(old_colony.second);i!=e;++i)
		{
			if(*i == c)
			{
				old_host.planets.planet_erase(old_colony.second,i);
				break;
			}
		}
	}
}

void GameState::trade_ship_for_tick_tock_token(const PlayerColors c)
{
	assert(get_player(c).alien->get_name().compare("Tick-Tock") == 0 && "An alien other than Tick-Tock used the Tick-Tock super flare!");

	std::stringstream prompt;
	prompt << "Send one of your ships to the warp to discard a Tick-Tock token?";
	std::vector<std::string> options;
	options.push_back("Y");
	options.push_back("N");

	unsigned choice = prompt_player(c,prompt.str(),options);

	if(choice == 1)
	{
		return;
	}

	//TODO: Have this function return a bool to tell us if a ship was successfully lost? From my interpretation of the Tick-Tock super, if the player doesn't have any ships to send to the warp then they shouldn't be able to discard a token
	lose_ships_to_warp(c,1);

	TickTock *t = dynamic_cast<TickTock*>(get_player(c).alien.get());
	t->discard_token();
	unsigned num_remaining = t->get_tokens();
	update_tick_tock_tokens(num_remaining);
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
	}

	return choice;
}

void GameState::dump_current_stack() const
{
	std::stack<GameEvent> copy_stack = stack;

	std::stringstream announce;
	announce << "[stack_update]\n";
	while(!copy_stack.empty())
	{
		GameEvent g = copy_stack.top();
		unsigned depth = copy_stack.size()-1;
		announce << "{" << depth << ": " << to_string(g.player) << " -> " << to_string(g.event_type);
		if(!g.aux.empty())
		{
			announce << " " << g.aux;
		}
		announce << "}\n";
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
	if(assignments.get_defense() != PlayerColors::Invalid)
	{
		player_order.push_back(assignments.get_defense()); //Then defense, if the defense has been assigned
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
		std::vector<GameEvent> valid_plays;
		current_player.can_respond(state,g,valid_plays); //Fill up valid_plays with potential GameEvent responses
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
	dump_current_stack();
}

