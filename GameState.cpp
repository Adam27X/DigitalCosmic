#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <limits>

#include "GameState.hpp"

GameState::GameState(unsigned nplayers) : num_players(nplayers), players(nplayers), destiny_deck(DestinyDeck(nplayers)), invalidate_next_callback(false), player_to_be_plagued(max_player_sentinel)
{
	assert(nplayers > 1 && nplayers < max_player_sentinel && "Invalid number of players!");
	std::cout << "Starting Game with " << num_players << " players\n";

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

	for(unsigned i=0; i<players.size(); i++)
	{
		players[i].set_game_state(this);
	}
}

void GameState::dump() const
{
	dump_planets();
	dump_PlanetInfo(hyperspace_gate,"Hyperspace gate");
	dump_PlanetInfo(warp,"Warp");
}

void GameState::dump_planets() const
{
	std::cout << "Current scores:\n";
	for(auto i=players.begin(),e=players.end();i!=e;++i)
	{
		std::cout << to_string(i->color) << " Player score: " << i->score << "\n";
		std::cout << "Planets: {";
		for(auto ii=i->planets.begin(),ee=i->planets.end();ii!=ee;++ii)
		{
			if(ii != i->planets.begin())
				std::cout << ",";
			std::cout << "{";
			for(auto iii=ii->begin(),eee=ii->end();iii!=eee;++iii)
			{
				if(iii != ii->begin())
					std::cout << ",";
				std::cout << to_string(*iii);
			}
			std::cout << "}";
		}
		std::cout << "}\n";
	}
	std::cout << "\n";
}

void GameState::dump_PlanetInfo(const PlanetInfo &source, const std::string name) const
{
	if(source.size())
	{
		std::cout << name << " :{";
	}
	for(auto i=source.begin(),e=source.end();i!=e;++i)
	{
		if(i!=source.begin())
			std::cout << ",";
		std::cout << to_string(*i);
	}
	if(source.size())
	{
		std::cout << "}\n";
	}
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
		players[player_to_be_dealt_this_card].hand.push_back(*iter); //Place the card in the player's hand
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
	player.hand.push_back(*iter);
	cosmic_deck.erase(iter);

	GameEvent g(player.color,GameEventType::DrawCard);
	resolve_game_event(g);
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
	std::cout << "The " << to_string(first_player) << " player will go first\n";
	assignments.offense = first_player;
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

void GameState::discard_and_draw_new_hand(PlayerInfo &player)
{
	//Copy cards from the player's hand to discard
	for(auto i=player.hand.begin(),e=player.hand.end();i!=e;++i)
	{
		cosmic_discard.push_back(*i);
	}
	player.hand.clear();

	if(cosmic_deck.size() < 8)
	{
		//Move remaining cards in the deck to the player, then move discard to the deck, then shuffle and finish dealing the player's new hand
		for(auto i=cosmic_deck.begin(),e=cosmic_deck.end();i!=e;++i)
		{
			player.hand.push_back(*i);
		}
		cosmic_deck.clear();
		shuffle_discard_into_cosmic_deck();
	}

	while(player.hand.size() < 8)
	{
		player.hand.push_back(*cosmic_deck.begin());
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
	for(auto i=warp.begin(),e=warp.end();i!=e;++i)
	{
		move_ship_to_colony(get_player(*i),warp);
	}
}

void GameState::cast_plague(const PlayerColors casting_player)
{
	std::cout << "Which player would you like to harm?\n";
	for(unsigned i=0; i<players.size(); i++)
	{
		std::cout << i << ": " << to_string(players[i].color) << "\n";
	}

	unsigned chosen_option;
	do
	{
		std::cout << "Please choose one of the option numbers above.\n";
		std::cout << to_string(casting_player) << ">>";
		std::cin >> chosen_option;
	} while(chosen_option > players.size());

	assert(player_to_be_plagued == max_player_sentinel);
	player_to_be_plagued = chosen_option;
}

void GameState::cast_force_field(const PlayerColors casting_player)
{
	std::cout << "Which alliances would you like to end?\n";
	for(auto i=assignments.offensive_allies.begin(),e=assignments.offensive_allies.end();i!=e;++i)
	{
		std::cout << to_string(i->first) << "? y/n (allied with the offense)\n";
		std::string response;
		do
		{
			std::cout << to_string(casting_player) << ">>";
			std::cin >> response;
		} while(response.compare("y") != 0 && response.compare("n") != 0);

		if(response.compare("y") == 0)
		{
			allies_to_be_stopped.insert(i->first);
		}
	}

	for(auto i=assignments.defensive_allies.begin(),e=assignments.defensive_allies.end();i!=e;++i)
	{
		std::cout << to_string(i->first) << "? y/n (allied with the defense)\n";
		std::string response;
		do
		{
			std::cout << to_string(casting_player) << ">>";
			std::cin >> response;
		} while(response.compare("y") != 0 && response.compare("n") != 0);

		if(response.compare("y") == 0)
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
					hyperspace_gate.erase(i);
					break;
				}
			}
		}
		else
		{
			const std::pair<PlayerColors,unsigned> chosen_colony = prompt_valid_colonies(player,valid_colonies);
			PlayerInfo &player_with_chosen_colony = get_player(chosen_colony.first);
			for(auto i=player_with_chosen_colony.planets[chosen_colony.second].begin(),e=player_with_chosen_colony.planets[chosen_colony.second].end();i!=e;++i)
			{
				if(*i == player)
				{
					warp.push_back(*i);
					player_with_chosen_colony.planets[chosen_colony.second].erase(i);
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
	for(auto i=victim.hand.begin(),e=victim.hand.end();i!=e;++i)
	{
		if((*i >= CosmicCardType::CardZap) && (*i <= CosmicCardType::Quash))
		{
			valid_artifacts.push_back(*i);
		}
	}
	if(valid_artifacts.size())
	{
		std::cout << "The " << to_string(victim.color) << " player has the following artifacts. Choose one to discard.\n";
		for(unsigned i=0; i<valid_artifacts.size(); i++)
		{
			std::cout << i << ": " << to_string(valid_artifacts[i]) << "\n";
		}
		do
		{
			std::cout << "Please choose one of the option numbers above.\n";
			std::cout << to_string(victim.color) << ">>";
			std::cin >> chosen_option;
		} while(chosen_option >= valid_artifacts.size());
		CosmicCardType choice = valid_artifacts[chosen_option];
		for(auto i=victim.hand.begin(),e=victim.hand.end();i!=e;++i)
		{
			if(*i == choice)
			{
				cosmic_discard.push_back(*i);
				victim.hand.erase(i);
				break;
			}
		}
	}
	//Attack
	std::vector<CosmicCardType> valid_attacks;
	for(auto i=victim.hand.begin(),e=victim.hand.end();i!=e;++i)
	{
		if((*i >= CosmicCardType::Attack0) && (*i <= CosmicCardType::Attack40))
		{
			valid_attacks.push_back(*i);
		}
	}
	if(valid_attacks.size())
	{
		std::cout << "The " << to_string(victim.color) << " player has the following attack cards. Choose one to discard.\n";
		for(unsigned i=0; i<valid_attacks.size(); i++)
		{
			std::cout << i << ": " << to_string(valid_attacks[i]) << "\n";
		}
		do
		{
			std::cout << "Please choose one of the option numbers above.\n";
			std::cout << to_string(victim.color) << ">>";
			std::cin >> chosen_option;
		} while(chosen_option >= valid_attacks.size());
		CosmicCardType choice = valid_attacks[chosen_option];
		for(auto i=victim.hand.begin(),e=victim.hand.end();i!=e;++i)
		{
			if(*i == choice)
			{
				cosmic_discard.push_back(*i);
				victim.hand.erase(i);
				break;
			}
		}
	}
	//Negotiate
	for(auto i=victim.hand.begin(),e=victim.hand.end();i!=e;++i)
	{
		if(*i == CosmicCardType::Negotiate)
		{
			cosmic_discard.push_back(*i);
			victim.hand.erase(i);
			break;
		}
	}
	//Morph
	for(auto i=victim.hand.begin(),e=victim.hand.end();i!=e;++i)
	{
		if(*i == CosmicCardType::Morph)
		{
			cosmic_discard.push_back(*i);
			victim.hand.erase(i);
			break;
		}
	}
	//Reinforcement
	std::vector<CosmicCardType> valid_reinforcements;
	for(auto i=victim.hand.begin(),e=victim.hand.end();i!=e;++i)
	{
		if((*i >= CosmicCardType::Reinforcement2) && (*i <= CosmicCardType::Reinforcement5))
		{
			valid_reinforcements.push_back(*i);
		}
	}
	if(valid_reinforcements.size())
	{
		std::cout << "The " << to_string(victim.color) << " player has the following reinforcement cards. Choose one to discard.\n";
		for(unsigned i=0; i<valid_reinforcements.size(); i++)
		{
			std::cout << i << ": " << to_string(valid_reinforcements[i]) << "\n";
		}
		do
		{
			std::cout << "Please choose one of the option numbers above.\n";
			std::cout << to_string(victim.color) << ">>";
			std::cin >> chosen_option;
		} while(chosen_option >= valid_reinforcements.size());
		CosmicCardType choice = valid_reinforcements[chosen_option];
		for(auto i=victim.hand.begin(),e=victim.hand.end();i!=e;++i)
		{
			if(*i == choice)
			{
				cosmic_discard.push_back(*i);
				victim.hand.erase(i);
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

//TODO: Is this a good function to check player scores and aliens to see if someone won the game?
void GameState::check_for_game_events(PlayerInfo &offense)
{
	std::vector<GameEvent> valid_plays;

	unsigned player_index = max_player_sentinel;
	for(unsigned i=0; i<players.size(); i++)
	{
		if(players[i].color == offense.color)
		{
			player_index = i;
			break;
		}
	}
	const unsigned int initial_player_index = player_index;

	do
	{
		PlayerInfo &current_player = players[player_index];
		//Starting with the offense, check for valid plays (artifact cards or alien powers) based on the current TurnPhase
		valid_plays.clear();
		for(auto i=current_player.hand.begin(),e=current_player.hand.end(); i!=e; ++i)
		{
			if(can_play_card_with_empty_stack(state,*i,current_player.current_role))
			{
				GameEvent g(current_player.color,to_game_event_type(*i));
				valid_plays.push_back(g);
			}
		}

		//TODO: Handle events where we *must* use the Alien power
		GameEvent alien_power = current_player.can_use_alien_with_empty_stack(state);
		if(alien_power.event_type != GameEventType::None)
		{
			assert(alien_power.event_type == GameEventType::AlienPower);
			valid_plays.push_back(alien_power);
		}

		//List the valid plays and ask the player if they would like to do any. Note that if they choose one they can still play another
		while(valid_plays.size())
		{
			std::stringstream prompt;
			prompt << "The " << to_string(current_player.color) << " player has the following valid plays to choose from:\n";
			for(unsigned i=0; i<valid_plays.size(); i++)
			{
				prompt << "Option " << i << ": " << to_string(valid_plays[i].event_type) << "\n";
			}
			prompt << "Option " << valid_plays.size() << ": None\n";
			unsigned chosen_option;
			do
			{
				std::cout << prompt.str();
				std::cout << "Please choose one of the option numbers above.\n";
				std::cout << to_string(current_player.color) << ">>";
				std::cin >> chosen_option;
			} while(chosen_option > valid_plays.size()); //TODO: What's the proper way to protect bad input here? We don't want to crash, we just want to retry the prompt. Take in a string instead and filter it

			if(chosen_option != valid_plays.size()) //An action was taken
			{
				GameEvent g = valid_plays[chosen_option];
				if(g.event_type != GameEventType::AlienPower)
				{
					CosmicCardType play = to_cosmic_card_type(g.event_type);

					//Remove this card from the player's hand and add it to discard
					add_to_discard_pile(play);
					unsigned old_hand_size = current_player.hand.size(); //Sanity checking
					for(auto i=current_player.hand.begin(),e=current_player.hand.end(); i!=e; ++i)
					{
						if(*i == play)
						{
							current_player.hand.erase(i);
							break;
						}
					}
					assert(current_player.hand.size()+1 == old_hand_size && "Error removing played card from the player's hand!");

					get_callbacks_for_cosmic_card(play,g);
				}

				if(g.callback_if_action_taken)
					g.callback_if_action_taken();
				g.callback_if_action_taken = nullptr;
				resolve_game_event(g);

				//Remove this option from valid_plays and if there are still plays that could be made, prompt them again
				unsigned valid_play_index = 0;
				for(auto i=valid_plays.begin(),e=valid_plays.end();i!=e;++i)
				{
					if(valid_play_index == chosen_option)
					{
						valid_plays.erase(i);
						break;
					}
					valid_play_index++;
				}
			}
			else
			{
				break;
			}
		}

		player_index = (player_index+1) % players.size();
	} while(player_index != initial_player_index);
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

		default:
			assert(0 && "CosmicCardType callbacks not yet implemenmted\n");
		break;
	}
}

void GameState::draw_from_destiny_deck()
{
	DestinyCardType dest = destiny_deck.draw();
	const PlayerColors off = assignments.offense;

	if(to_PlayerColors(dest) != PlayerColors::Invalid)
	{
		//We drew a player color
		PlayerColors drawn = to_PlayerColors(dest);

		if(drawn == off)
		{
			std::cout << "The " << to_string(off) << " player has drawn his or her own color.\n";

			PlayerInfo &offense = get_player(off);
			std::map< std::pair<PlayerColors,unsigned>, unsigned > valid_home_system_encounters; //Map <opponent,planet number> -> number of opponent ships on that planet
			for(unsigned i=0; i<offense.planets.size(); i++)
			{
				for(auto ii=offense.planets[i].begin(),ee=offense.planets[i].end(); ii!=ee; ++ii)
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
				std::cout << "Since there are no opponents in the offense's home system the offense must draw another destiny card\n";
				draw_from_destiny_deck();
			}
			else
			{
				std::cout << "The offense may either have an encounter with a player in his or her own system or draw another destiny card\n";
				unsigned option = 0;
				//TODO: It's probably better to let the offense choose between some sort of home system encounter and redrawing now and then letting them choose the specific home system encounter during launch should they go that route
				//	In practice I'm not sure that this timing will ever matter
				for(auto i=valid_home_system_encounters.begin(),e=valid_home_system_encounters.end();i!=e;++i)
				{
					std::cout << option << ": Have an encounter with the " << to_string(i->first.first) << " player on " << to_string(off) << " Planet " << i->first.second << " (the " << to_string(i->first.first) << " player has " << i->second << " ships on this planet.)\n";
					option++;
				}
				std::cout << option << ": Draw another destiny card\n";
				unsigned chosen_option;
				do
				{
					std::cout << "Please choose one of the above options.\n";
					std::cout << to_string(off) << ">>";
					std::cin >> chosen_option;
				} while(chosen_option > valid_home_system_encounters.size());

				if(chosen_option == valid_home_system_encounters.size())
				{
					std::cout << "The offense has chosen to redraw\n";
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
				num_ships_in_warp[*i]++;
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
				if((current_player_color != off) && (players[current_player_index].hand.size() > most_cards_in_hand))
				{
					most_cards_in_hand = players[current_player_index].hand.size();
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
			std::cout << "The offense has drawn a wild destiny card and may have an encounter with the player of his or her choice in their home system.\n";
			for(unsigned i=0; i<players.size(); i++)
			{
				if(players[i].color != off)
				{
					std::cout << i << ": " << to_string(players[i].color) << "\n";
				}
			}
			unsigned chosen_option;
			do
			{
				std::cout << "Please choose which player you would like to attack.\n";
				std::cout << to_string(off) << ">>";
				std::cin >> chosen_option;
			} while((chosen_option >= players.size()) || (players[chosen_option].color == off));

			assignments.defense = players[chosen_option].color;
			assignments.planet_location = players[chosen_option].color;
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
	if(assignments.planet_location == assignments.offense)
		return;

	std::cout << "The offense has chosen to have an encounter with the " << to_string(assignments.defense) << " player.\n";
	const PlayerInfo &host = get_player(assignments.planet_location);
	unsigned chosen_option;
	do
	{
		//TODO: Dump planet info here to help the player make his or her choice
		std::cout << "Please choose which planet you would like to attack (0-" << host.planets.size()-1 << ").\n";
		std::cout << to_string(assignments.offense) << ">>";
		std::cin >> chosen_option;
	} while(chosen_option > (host.planets.size()-1));
	assignments.planet_id = chosen_option;
}

void GameState::send_in_ships(const PlayerColors player)
{
	//List the player's valid colonies and have them choose a colony or none until they've chosen none or they've chosen four
	std::cout << "The " << to_string(player) << " player can choose up to four ships from any of their valid colonies\n";
	unsigned launched_ships = 0;
	unsigned choice;
	std::vector< std::pair<PlayerColors,unsigned> > valid_colonies;
	do
	{
		valid_colonies = get_valid_colonies(player);
		for(unsigned i=0; i<valid_colonies.size(); i++)
		{
			std::cout << i << ": " << to_string(valid_colonies[i].first) << " planet " << valid_colonies[i].second << "\n";
		}
		std::cout << valid_colonies.size() << ": None\n";

		std::cout << "Please choose one of the above options.\n";
		std::cout << to_string(player) << ">>";
		std::cin >> choice;
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
			for(auto i=host.planets[planet_id].begin(),e=host.planets[planet_id].end();i!=e;++i)
			{
				if(*i == player)
				{
					host.planets[planet_id].erase(i);
					break;
				}
			}
			launched_ships++;
		}
	} while(((choice < valid_colonies.size()) && launched_ships < 4) || choice > valid_colonies.size());

	//FIXME: Does the offense *have* to send in a ship if they have one?
	if((player != assignments.offense) && (launched_ships == 0))
	{
		std::cout << "Allies *must* commit at least one ship to the encounter. Try again.\n";
		send_in_ships(player);
	}
}

std::set<PlayerColors> GameState::invite_allies(const std::set<PlayerColors> &potential_allies, bool offense)
{
	std::set<PlayerColors> invited;
	std::string inviter = offense ? "offense" : "defense";
	for(auto i=potential_allies.begin(),e=potential_allies.end();i!=e;++i)
	{
		std::string response;
		do
		{
			std::cout << "Would the " << inviter << " like to invite the " << to_string(*i) << " player as an ally? y/n\n";
			if(offense)
				std::cout << to_string(assignments.offense) << ">>";
			else
				std::cout << to_string(assignments.defense) << ">>";
			std::cin >> response;
		} while(response.compare("y") != 0 && response.compare("n") != 0);

		if(response.compare("y") == 0)
		{
			invited.insert(*i);
		}
	}

	return invited;
}

void GameState::form_alliances(std::set<PlayerColors> &invited_by_offense, std::set<PlayerColors> &invited_by_defense)
{
	unsigned player_index = max_player_sentinel;
	//FIXME: Write a PlayerColors -> PlayerID helper because this is used all over the place. Could even write an iterator that takes in a function to be executed in player order
	for(unsigned i=0; i<players.size(); i++)
	{
		if(players[i].color == assignments.offense)
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
			std::string response;
			do
			{
				std::cout << "Would the " << to_string(current_player_color) << " like to join with the offense? y/n\n";
				std::cout << to_string(current_player_color) << ">>";
				std::cin >> response;
			} while(response.compare("y") != 0 && response.compare("n") != 0);

			if(response.compare("y") == 0)
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
			std::string response;
			do
			{
				std::cout << "Would the " << to_string(current_player_color) << " like to join with the defense? y/n\n";
				std::cout << to_string(current_player_color) << ">>";
				std::cin >> response;
			} while(response.compare("y") != 0 && response.compare("n") != 0);

			if(response.compare("y") == 0)
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
	PlayerInfo &offense = get_player(assignments.offense);
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
	std::string response;
	do
	{
		std::cout << "Was the deal successful?\n";
		std::cout << to_string(assignments.offense) << "/" << to_string(assignments.defense) << ">>";
		std::cin >> response;
	} while(response.compare("y") != 0 && response.compare("n") != 0);

	if(response.compare("y") == 0)
	{
		deal_params.successful = true;
	}
	response = "";

	if(deal_params.successful) //Collect the terms of the deal
	{
		std::vector< std::pair<PlayerColors,unsigned> > defense_valid_colonies = get_valid_colonies(assignments.defense); //A list of planet colors and indices
		std::vector< std::pair<PlayerColors,unsigned> > offense_valid_colonies = get_valid_colonies(assignments.offense); //A list of planet colors and indices

		if(defense_valid_colonies.empty())
		{
			std::cout << "Note: The defense has no valid colonies and therefore cannot allow the offense to establish a colony!\n";
		}
		else
		{
			do
			{
				std::cout << "Will the " << to_string(assignments.offense) << " player (offense) establish a colony on any one planet where the " << to_string(assignments.defense) << " player (defense) has a colony? y/n\n";
				std::cout << to_string(assignments.offense) << "/" << to_string(assignments.defense) << ">>";
				std::cin >> response;
			} while(response.compare("y") != 0 && response.compare("n") != 0);

			if(response.compare("y") == 0)
			{
				deal_params.offense_receives_colony = true;
			}
			response = "";
		}

		if(offense_valid_colonies.empty())
		{
			std::cout << "Note: The offense has no valid colonies and therefore cannot allow the defense to establish a colony!\n";
		}
		else
		{
			do
			{
				std::cout << "Will the " << to_string(assignments.defense) << " player (defense) establish a colony on any one planet where the " << to_string(assignments.offense) << " player (offense) has a colony? y/n\n";
				std::cout << to_string(assignments.offense) << "/" << to_string(assignments.defense) << ">>";
				std::cin >> response;
			} while(response.compare("y") != 0 && response.compare("n") != 0);

			if(response.compare("y") == 0)
			{
				deal_params.defense_receives_colony = true;
			}
			response = "";
		}

		std::cout << "How many cards will the " << to_string(assignments.offense) << " (offense) receive from the " << to_string(assignments.defense) << " player (defense)?\n";
		std::cout << to_string(assignments.offense) << "/" << to_string(assignments.defense) << ">>";
		std::cin >> deal_params.num_cards_to_offense;

		if(deal_params.num_cards_to_offense > 0)
		{
			do
			{
				std::cout << "Will these cards be chosen randomly? (If not they will be chosen by the " << to_string(assignments.defense) << " player (defense).\n";
				std::cout << to_string(assignments.offense) << "/" << to_string(assignments.defense) << ">>";
				std::cin >> response;
			} while(response.compare("y") != 0 && response.compare("n") != 0);

			if(response.compare("y") == 0)
			{
				deal_params.cards_to_offense_chosen_randomly = true;
			}
			response = "";
		}

		std::cout << "How many cards will the " << to_string(assignments.defense) << " (defense) receive from the " << to_string(assignments.offense) << " player (offense)?\n";
		std::cout << to_string(assignments.offense) << "/" << to_string(assignments.defense) << ">>";
		std::cin >> deal_params.num_cards_to_defense;

		if(deal_params.num_cards_to_defense > 0)
		{
			do
			{
				std::cout << "Will these cards be chosen randomly? (If not they will be chosen by the " << to_string(assignments.offense) << " player (offense).\n";
				std::cout << to_string(assignments.offense) << "/" << to_string(assignments.defense) << ">>";
				std::cin >> response;
			} while(response.compare("y") != 0 && response.compare("n") != 0);

			if(response.compare("y") == 0)
			{
				deal_params.cards_to_defense_chosen_randomly = true;
			}
		}

		//TODO: Add a check to ensure that something was exchange? Otherwise you could force a failed deal or re-prompt the players...
	}
}

void GameState::resolve_negotiation()
{
	if(deal_params.successful)
	{
		//Now that we have the information we need, carry out the deal
		std::vector< std::pair<PlayerColors,unsigned> > defense_valid_colonies = get_valid_colonies(assignments.defense); //A list of planet colors and indices
		std::vector< std::pair<PlayerColors,unsigned> > offense_valid_colonies = get_valid_colonies(assignments.offense); //A list of planet colors and indices
		if(deal_params.offense_receives_colony)
		{
			assert(!defense_valid_colonies.empty());
			//Choose from any of the valid defense colonies
			const std::pair<PlayerColors,unsigned> chosen_colony = prompt_valid_colonies(assignments.offense,defense_valid_colonies);
			PlayerInfo &player_with_chosen_colony = get_player(chosen_colony.first);
			player_with_chosen_colony.planets[chosen_colony.second].push_back(assignments.offense);
		}
		if(deal_params.defense_receives_colony)
		{
			assert(!offense_valid_colonies.empty());
			//Choose from any of the valid offense colonies
			const std::pair<PlayerColors,unsigned> chosen_colony = prompt_valid_colonies(assignments.defense,offense_valid_colonies);
			PlayerInfo &player_with_chosen_colony = get_player(chosen_colony.first);
			player_with_chosen_colony.planets[chosen_colony.second].push_back(assignments.defense);
		}
		std::vector<CosmicCardType> cards_to_offense;
		//Choose which cards will be taken from the defense
		PlayerInfo &defense = get_player(assignments.defense);
		if(deal_params.num_cards_to_offense > 0)
		{
			assert(deal_params.num_cards_to_offense <= defense.hand.size());
			if(deal_params.cards_to_offense_chosen_randomly)
			{
				for(unsigned i=0; i<deal_params.num_cards_to_offense; i++)
				{
					unsigned chosen_card_index = rand() % defense.hand.size();
					cards_to_offense.push_back(defense.hand[chosen_card_index]);
					defense.hand.erase(defense.hand.begin()+chosen_card_index);
				}
			}
			else
			{
				for(unsigned i=0; i<deal_params.num_cards_to_offense; i++)
				{
					std::cout << "The " << to_string(assignments.defense) << " player has the following cards. Choose one to give to the offense.\n";
					for(unsigned i=0; i<defense.hand.size(); i++)
					{
						std::cout << i << ": " << to_string(defense.hand[i]) << "\n";
					}
					unsigned chosen_option;
					do
					{
						std::cout << "Please choose one of the option numbers above.\n";
						std::cout << to_string(assignments.defense) << ">>";
						std::cin >> chosen_option;
					} while(chosen_option >= defense.hand.size());
					cards_to_offense.push_back(defense.hand[chosen_option]);
					defense.hand.erase(defense.hand.begin()+chosen_option);
				}
			}
		}
		std::vector<CosmicCardType> cards_to_defense;
		//Choose which cards will be taken from the offense
		PlayerInfo &offense = get_player(assignments.offense);
		if(deal_params.num_cards_to_defense > 0)
		{
			assert(deal_params.num_cards_to_defense <= offense.hand.size());
			if(deal_params.cards_to_defense_chosen_randomly)
			{
				for(unsigned i=0; i<deal_params.num_cards_to_defense; i++)
				{
					unsigned chosen_card_index = rand() % offense.hand.size();
					cards_to_defense.push_back(offense.hand[chosen_card_index]);
					offense.hand.erase(offense.hand.begin()+chosen_card_index);
				}
			}
			else
			{
				for(unsigned i=0; i<deal_params.num_cards_to_defense; i++)
				{
					std::cout << "The " << to_string(assignments.offense) << " player has the following cards. Choose one to give to the defense.\n";
					for(unsigned i=0; i<offense.hand.size(); i++)
					{
						std::cout << i << ": " << to_string(offense.hand[i]) << "\n";
					}
					unsigned chosen_option;
					do
					{
						std::cout << "Please choose one of the option numbers above.\n";
						std::cout << to_string(assignments.offense) << ">>";
						std::cin >> chosen_option;
					} while(chosen_option >= offense.hand.size());
					cards_to_defense.push_back(offense.hand[chosen_option]);
					offense.hand.erase(offense.hand.begin()+chosen_option);
				}
			}
		}
		//Distribute the chosen cards to their new players
		for(auto i=cards_to_offense.begin(),e=cards_to_offense.end();i!=e;++i)
		{
			offense.hand.push_back(*i);
		}
		for(auto i=cards_to_defense.begin(),e=cards_to_defense.end();i!=e;++i)
		{
			defense.hand.push_back(*i);
		}
	}
	else //Unsuccessful deal, each player loses 3 ships to the warp
	{
		//NOTE: This signifies an unsuccessful encounter
		//This logic should be similar to part of a plague
		lose_ships_to_warp(assignments.offense,3);
		lose_ships_to_warp(assignments.defense,3);
	}
}

void GameState::setup_compensation()
{
	//The player with the negotiate loses, but collects compensation later
	if(assignments.offensive_encounter_card == CosmicCardType::Negotiate)
	{
		assignments.player_receiving_compensation = assignments.offense;
		assignments.player_giving_compensation = assignments.defense;
	}
	else
	{
		assignments.player_receiving_compensation = assignments.defense;
		assignments.player_giving_compensation = assignments.offense;
	}

	std::cout << "The " << to_string(assignments.player_receiving_compensation) << " player has lost the encoutner, but will receive compensation from the " << to_string(assignments.player_giving_compensation) << " player.\n";
}

void GameState::resolve_compensation()
{
	//The losing player's ships go to the warp
	unsigned number_of_ships_lost = 0;
	if(assignments.player_receiving_compensation == assignments.offense)
	{
		for(auto i=hyperspace_gate.begin();i!=hyperspace_gate.end();)
		{
			if(*i == assignments.player_receiving_compensation)
			{
				warp.push_back(*i);
				i=hyperspace_gate.erase(i);
				number_of_ships_lost++;
			}
			else
			{
				++i;
			}
		}
	}
	else
	{
		PlanetInfo &encounter_planet = get_player(assignments.planet_location).planets[assignments.planet_id];
		for(auto i=encounter_planet.begin();i!=encounter_planet.end();)
		{
			if(*i == assignments.player_receiving_compensation)
			{
				warp.push_back(*i);
				i = encounter_planet.erase(i);
				number_of_ships_lost++;
			}
			else
			{
				++i;
			}
		}

		//Move the offensive ships to their newly established colony
		for(auto i=hyperspace_gate.begin();i!=hyperspace_gate.end();)
		{
			if(*i == assignments.offense)
			{
				encounter_planet.push_back(*i);
				i=hyperspace_gate.erase(i);
			}
			else
			{
				++i;
			}
		}
	}

	//For each ship lost, the losing player receives a card from the opponent's hand chosen at random
	PlayerInfo &player_giving_compensation = get_player(assignments.player_giving_compensation);
	PlayerInfo &player_receiving_compensation = get_player(assignments.player_receiving_compensation);
	for(unsigned i=0; i<number_of_ships_lost; i++)
	{
		if(player_giving_compensation.hand.empty())
			break;

		unsigned card_choice = rand() % player_giving_compensation.hand.size();
		player_receiving_compensation.hand.push_back(player_giving_compensation.hand[card_choice]);
		player_giving_compensation.hand.erase(player_giving_compensation.hand.begin()+card_choice);
	}
}

void GameState::setup_attack()
{
	//Base values from encounter cards
	if(assignments.offensive_encounter_card == CosmicCardType::Morph && assignments.defensive_encounter_card == CosmicCardType::Morph)
	{
		//There's only one Morph card in the deck, but the rules explicitly state that *both* players lose the encounter should this scenario somehow occur
		std::cout << "Both players revealed a Morph card! Both players lose the encounter and all ships involved will go to the warp. Yes, this scenario is explicitly laid out in the rules.\n";
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

	std::cout << "Initial scores from encounter cards: Offense = " << assignments.offense_attack_value << "; Defense = " << assignments.defense_attack_value << "\n";

	//Add score from ships
	for(auto i=hyperspace_gate.begin(),e=hyperspace_gate.end(); i!=e; ++i)
	{
		if(*i == assignments.offense || assignments.offensive_allies.find(*i) != assignments.offensive_allies.end())
		{
			std::cout << "Adding " << to_string(*i) << " ship from hyperspace gate to offense score.\n";
			assignments.offense_attack_value++;
		}
	}

	const PlanetInfo &encounter_planet = get_player(assignments.planet_location).planets[assignments.planet_id];
	for(auto i=encounter_planet.begin(),e=encounter_planet.end();i!=e;++i)
	{
		if(*i == assignments.defense)
		{
			std::cout << "Adding " << to_string(*i) << " ship from the colony being attacked to defense score.\n";
			assignments.defense_attack_value++;
		}
	}
	for(auto i=assignments.defensive_allies.begin(),e=assignments.defensive_allies.end();i!=e;++i)
	{
		std::cout << "Adding " << i->second << " ship(s) from the " << to_string(i->first) << " defensive ally.\n";
		assignments.defense_attack_value += i->second;
	}

	std::cout << "Total score after adding ships (ties go to the defense): Offense = " << assignments.offense_attack_value << "; Defense = " << assignments.defense_attack_value << "\n";
}

void GameState::resolve_attack()
{
	if(assignments.offensive_encounter_card == CosmicCardType::Morph && assignments.defensive_encounter_card == CosmicCardType::Morph)
	{
		//Send all ships involved to the warp
		for(auto i=hyperspace_gate.begin();i!=hyperspace_gate.end();)
		{
			warp.push_back(*i);
			i=hyperspace_gate.erase(i);
		}
		PlanetInfo &encounter_planet = get_player(assignments.planet_location).planets[assignments.planet_id];
		for(auto i=encounter_planet.begin();i!=encounter_planet.end();)
		{
			if(*i == assignments.defense)
			{
				warp.push_back(*i);
				i = encounter_planet.erase(i);
			}
			else
			{
				++i;
			}
		}
		for(auto i=defensive_ally_ships.begin();i!=defensive_ally_ships.end();)
		{
			warp.push_back(*i);
			i=defensive_ally_ships.erase(i);
		}

		return;
	}

	if(assignments.offense_attack_value > assignments.defense_attack_value)
	{
		//Defense and defensive ally ships go to the warp
		PlanetInfo &encounter_planet = get_player(assignments.planet_location).planets[assignments.planet_id];
		for(auto i=encounter_planet.begin();i!=encounter_planet.end();)
		{
			if(*i == assignments.defense)
			{
				warp.push_back(*i);
				i = encounter_planet.erase(i);
			}
			else
			{
				++i;
			}
		}
		for(auto i=defensive_ally_ships.begin();i!=defensive_ally_ships.end();)
		{
			warp.push_back(*i);
			i=defensive_ally_ships.erase(i);
		}

		//Offense and offensive ally ships create (or reinforce) a colony on the encounter planet
		for(auto i=hyperspace_gate.begin();i!=hyperspace_gate.end();)
		{
			encounter_planet.push_back(*i);
			i=hyperspace_gate.erase(i);
		}

		//TODO: Since the offense won, they may have a second encounter (double check to see if this is actually optional)

	}
	else //Ties go to the defense
	{
		//Offense and offensive ally ships go to the warp
		for(auto i=hyperspace_gate.begin();i!=hyperspace_gate.end();)
		{
			warp.push_back(*i);
			i=hyperspace_gate.erase(i);
		}
		//Defensive allies return each of their ships to one of their colonies
		while(!defensive_ally_ships.empty())
		{
			move_ship_to_colony(get_player(*defensive_ally_ships.begin()),defensive_ally_ships);
		}
		//Handle defender rewards (Note: This should generate quite a few Remora triggers)
		for(auto i=assignments.defensive_allies.begin(),e=assignments.defensive_allies.end();i!=e;++i)
		{
			unsigned num_ships = i->second;
			for(unsigned ship=0; ship<num_ships; ship++)
			{
				//For each ship, the ally gets the choice of returning a ship from the warp or drawing a card
				bool ship_exists_in_warp = false;
				for(auto ii=warp.begin(),ee=warp.end();ii!=ee;++ii)
				{
					if(*ii == i->first)
					{
						ship_exists_in_warp = true;
					}
				}

				if(!ship_exists_in_warp) //The player has no ships in the warp and must draw a card
				{
					std::cout << "Defender rewards. Drawing a card for the " << to_string(i->first) << " player.\n";
					draw_cosmic_card(get_player(i->first));
				}
				else
				{
					unsigned choice;
					do
					{
						std::cout << "Defender rewards. Choose one of the options below.\n";
						std::cout << "0: Retrieve one of your ships from the warp.\n";
						std::cout << "1: Draw a card from the Cosmic deck.\n";
						std::cout << to_string(i->first) << ">>";
						std::cin >> choice;
					} while(choice != 0 && choice != 1);

					if(choice == 0)
					{
						move_ship_to_colony(get_player(i->first),warp);
					}
					else
					{
						draw_cosmic_card(get_player(i->first));
					}
				}
			}
		}

		//TODO: Since the offense lost, they do not have a second encounter
	}
}

void GameState::execute_turn()
{
	//Start Turn Phase
	state = TurnPhase::StartTurn;
	for(unsigned i=0; i<players.size(); i++)
	{
		//FIXME: update this data appropriately or infer it from assignments
		players[i].current_role = EncounterRole::None;
	}

	//Ensure the offense has a valid hand
	PlayerInfo &offense = get_player(assignments.offense);
	offense.current_role = EncounterRole::Offense;
	bool offense_needs_discard = !offense.has_encounter_cards_in_hand();

	std::cout << "The " << to_string(assignments.offense) << " Player is now the offense.\n";
	if(offense_needs_discard)
	{
		std::cout << "The offense has no encounter cards in hand. They now must discard their hand and draw eight cards\n";
		discard_and_draw_new_hand(offense);

		//This implementations treats the dump and discard operation as one draw action, even if the player is forced to draw and discard multiple times
		//Hence Remora will only draw once for this action, which seems appropriate
		dump_player_hand(offense);
	}

	check_for_game_events(offense);

	//Regroup Phase
	state = TurnPhase::Regroup;

	//If the offense has any ships in the warp, they retrieve one of them
	for(auto i=warp.begin(),e=warp.end();i!=e;++i)
	{
		if(*i == offense.color)
		{
			std::cout << "The " << to_string(assignments.offense) << " player will now regroup\n";
			move_ship_to_colony(offense,warp);
			break;
		}
	}

	check_for_game_events(offense);

	//Destiny Phase
	state = TurnPhase::Destiny;

	draw_from_destiny_deck();

	check_for_game_events(offense);

	//Launch Phase
	state = TurnPhase::Launch;

	choose_opponent_planet();

	send_in_ships(assignments.offense);

	check_for_game_events(offense);

	//Alliance Phase
	state = TurnPhase::Alliance;

	//Offense invites, then the defense invites, then players accept/reject and commit ships in turn order
	std::set<PlayerColors> potential_allies;
	for(unsigned i=0; i<players.size(); i++)
	{
		if((players[i].color != assignments.offense) && (players[i].color != assignments.defense))
		{
			potential_allies.insert(players[i].color);
		}
	}

	std::set<PlayerColors> invited_by_offense = invite_allies(potential_allies,true);
	std::set<PlayerColors> invited_by_defense = invite_allies(potential_allies,false);

	form_alliances(invited_by_offense,invited_by_defense);

	check_for_game_events(offense);

	//Planning Phase
	state = TurnPhase::Planning_before_selection;

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

		//TODO: Refactor this out into an end of turn cleanup phase?
		assignments.clear(); //Reset assignments
		for(unsigned i=0; i<players.size(); i++)
		{
			players[i].current_role = EncounterRole::None;
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
	//TODO: Support Trader alien power
	check_for_game_events(offense);

	assignments.offensive_encounter_card = offense.choose_encounter_card();
	assignments.defensive_encounter_card = defense.choose_encounter_card();

	//After cards are selected effects can now resolve
	state = TurnPhase::Planning_after_selection;
	check_for_game_events(offense);

	//TODO: Reveal Phase
	state = TurnPhase::Reveal;

	std::cout << "The offense has encounter card: " << to_string(assignments.offensive_encounter_card) << "\n";
	std::cout << "The defense has encounter card: " << to_string(assignments.defensive_encounter_card) << "\n";

	//TODO: Support Human Alien power
	//TODO: Support the Emotion Control artifact card

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
		//TODO: Support reinforcement cards
		setup_attack();
	}

	check_for_game_events(offense);

	//TODO: Resolution Phase
	state = TurnPhase::Resolution;
	if(assignments.offensive_encounter_card == CosmicCardType::Negotiate && assignments.defensive_encounter_card == CosmicCardType::Negotiate)
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
}

void GameState::swap_encounter_cards()
{
	CosmicCardType tmp = assignments.offensive_encounter_card;
	assignments.offensive_encounter_card = assignments.defensive_encounter_card;
	assignments.defensive_encounter_card = tmp;
}

void GameState::swap_main_player_hands()
{
	PlayerInfo &offense = get_player(assignments.offense);
	PlayerInfo &defense = get_player(assignments.defense);

	std::vector<CosmicCardType> tmp = offense.hand;
	offense.hand = defense.hand;
	defense.hand = tmp;
}

//FIXME: This should be const
std::vector< std::pair<PlayerColors,unsigned> > GameState::get_valid_colonies(const PlayerColors color)
{
	std::vector< std::pair<PlayerColors,unsigned> > valid_colonies; //A list of planet colors and indices

	for(auto player_begin=players.begin(),player_end=players.end();player_begin!=player_end;++player_begin)
	{
		for(unsigned planet=0; planet<player_begin->planets.size(); planet++)
		{
			for(unsigned ships=0; ships<player_begin->planets[planet].size(); ships++)
			{
				if(player_begin->planets[planet][ships] == color) //If this ship matches our color we have a colony there
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
	for(unsigned i=0; i<valid_colonies.size(); i++)
	{
		prompt << "Option " << i << ": " << to_string(valid_colonies[i].first) << " Planet " << valid_colonies[i].second << "\n";
	}
	unsigned chosen_option;
	do
	{
		std::cout << prompt.str();
		std::cout << "Please choose one of the option numbers above.\n";
		std::cout << to_string(color) << ">>";
		std::cin >> chosen_option;
	} while(chosen_option >= valid_colonies.size()); //TODO: What's the proper way to protect bad input here? We don't want to crash, we just want to retry the prompt

	return valid_colonies[chosen_option];
}

//Source = warp, hyperspace_gate, defensive_ally_ships, etc.
void GameState::move_ship_to_colony(PlayerInfo &p, PlanetInfo &source)
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

	//If the player has no colonies the ship goes directly onto the hyperspace gate. It's unclear if this should really happen for players who aren't the offense, but it's a corner case and is a reasonable solution.
	//Chances are if you're even thinking about this scenario as player you are pretty screwed
	if(!valid_colonies.size())
	{
		std::cout << "The  " << to_string(p.color) << " player has no valid colonies! Placing the ship directly on the hyperspace gate.\n";
		hyperspace_gate.push_back(p.color);
	}
	else
	{
		const std::pair<PlayerColors,unsigned> chosen_colony = prompt_valid_colonies(p.color,valid_colonies);

		//Now actually add the colony
		bool colony_found = false; //paranoia
		for(auto player_begin=players.begin(),player_end=players.end();player_begin!=player_end;++player_begin)
		{
			if(player_begin->color != chosen_colony.first)
				continue;
			PlanetInfo &chosen_planet = player_begin->planets[chosen_colony.second];
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
				chosen_planet.push_back(p.color);
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

	if(&source == &warp) //If the source is the warp
	{
		GameEvent g(p.color,GameEventType::RetrieveWarpShip);
		resolve_game_event(g);
	}
}

std::string GameState::prompt_player(PlayerInfo &p, const std::string &prompt) const
{
	std::cout << prompt;
	std::cout << to_string(p.color) << ">>";
	std::string response;
	std::cin >> response;

	return response;
}

void GameState::dump_current_stack() const
{
	std::stack<GameEvent> copy_stack = stack;

	std::cout << "Current game stack:\n";
	while(!copy_stack.empty())
	{
		GameEvent g = copy_stack.top();
		unsigned depth = copy_stack.size()-1;
		std::cout << depth << ": " << to_string(g.player) << " -> " << to_string(g.event_type) << "\n";
		copy_stack.pop();
	}

	assert(copy_stack.empty() && "Error printing stack!");
}

void GameState::resolve_game_event(const GameEvent g)
{
	stack.push(g);
	dump_current_stack();

	//Enforce player order during resolution
	unsigned player_index = max_player_sentinel; //Sentinel value meant to be invalid
	for(unsigned i=0; i<players.size(); i++)
	{
		if(players[i].color == g.player)
		{
			player_index = i;
			break;
		}
	}
	const unsigned int initial_player_index = player_index;

	do
	{
		GameEvent can_respond = players[player_index].can_respond(state,g); //NOTE: This will eventually be a vector when multiple responses are valid
		if(can_respond.event_type != GameEventType::None) //If there is a valid response...
		{
			bool take_action = false;
			GameEvent must_respond = players[player_index].must_respond(state,g);
			if(must_respond.event_type != GameEventType::None)
			{
				std::cout << "The " << to_string(players[player_index].color) << " player *must* respond to the " << to_string(g.event_type) << " action.\n";
				take_action = true;
			}
			else
			{
				std::cout << "The " << to_string(players[player_index].color) << " player can respond to the " << to_string(g.event_type) << " action.\n";
				std::string response;
				do
				{
					std::stringstream response_prompt;
					response_prompt << "Would you like to respond to the " << to_string(g.event_type) << " with your " << to_string(can_respond.event_type) << "? y/n\n";
					response = prompt_player(players[player_index],response_prompt.str());
				} while(response.compare("y") != 0 && response.compare("n") != 0);
				if(response.compare("y") == 0)
				{
					take_action = true;
				}
			}

			if(take_action)
			{
				//FIXME: Check if must respond is valid and if so, take that action
				if(can_respond.callback_if_action_taken)
				{
					can_respond.callback_if_action_taken();
				}
				resolve_game_event(can_respond);
			}
		}

		player_index = (player_index+1) % players.size();
	} while(player_index != initial_player_index);

	if(g.callback_if_resolved)
	{
		if(invalidate_next_callback) //Countered!
		{
			invalidate_next_callback  = false;
		}
		else
		{
			g.callback_if_resolved();
		}
	}
	stack.pop();
}

void GameState::debug_send_ship_to_warp()
{
	PlayerColors victim = PlayerColors::Yellow;
	warp.push_back(victim);

	PlayerInfo &yellow = get_player(victim);
	yellow.planets[0].erase(yellow.planets[0].begin());

	PlayerColors victim2 = PlayerColors::Red;
	warp.push_back(victim2);
	PlayerInfo &red = get_player(victim2);
	red.planets[2].erase(red.planets[2].begin());
}

