#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>
#include <sstream>

#include "GameState.hpp"

GameState::GameState(unsigned nplayers) : num_players(nplayers), players(nplayers), destiny_deck(DestinyDeck(nplayers)), invalidate_next_callback(false)
{
	assert(nplayers > 1 && nplayers < 6 && "Invalid number of players!");
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

PlayerColors GameState::choose_first_player()
{
	return destiny_deck.draw_for_first_player_and_shuffle();
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
		discard_and_draw_new_hand(player);
}

void GameState::execute_turn(PlayerColors off)
{
	//Start Turn Phase
	state = TurnPhase::StartTurn;
	for(unsigned i=0; i<players.size(); i++)
	{
		players[i].current_role = EncounterRole::None;
	}

	//Ensure the offense has a valid hand
	PlayerInfo &offense = get_player(off);
	offense.current_role = EncounterRole::Offense;
	bool offense_needs_discard = !offense.has_encounter_cards_in_hand();

	std::cout << "The " << to_string(off) << " Player is now the offense.\n";
	if(offense_needs_discard)
	{
		std::cout << "The offense has no encounter cards in hand. They now must discard their hand and draw eight cards\n";
		discard_and_draw_new_hand(offense);

		//This implementations treats the dump and discard operation as one draw action, even if the player is forced to draw and discard multiple times
		//Hence Remora will only draw once for this action, which seems appropriate
		dump_player_hand(offense);

		GameEvent g(offense.color,GameEventType::DrawCard);
		resolve_game_event(g);
	}

	//Regroup Phase
	state = TurnPhase::Regroup;

	//If the offense has any ships in the warp, they retrieve one of them
	//NOTE: Remora can respond to this action if it is taken
	for(auto i=warp.begin(),e=warp.end();i!=e;++i)
	{
		if(*i == offense.color)
		{
			std::cout << "The " << to_string(off) << " player will now regroup\n";
			add_ship_to_colony(offense);
			warp.erase(i); //Remove the ship from the warp
			break;
		}
	}

	//TODO: Potentially add events if people want to play Mobius Tubes or Plague
}

//TODO: Test
void GameState::add_ship_to_colony(PlayerInfo &p)
{
	//Gather the valid options and present them to the player
	std::vector< std::pair<PlayerColors,unsigned> > valid_colonies; //A list of planet colors and indices

	for(auto player_begin=players.begin(),player_end=players.end();player_begin!=player_end;++player_begin)
	{
		for(unsigned planet=0; planet<player_begin->planets.size(); planet++)
		{
			for(unsigned ships=0; ships<player_begin->planets[planet].size(); ships++)
			{
				if(player_begin->planets[planet][ships] == p.color) //If this ship matches our color we have a colony there
				{
					valid_colonies.push_back(std::make_pair(player_begin->color,planet));
					break;
				}
			}
		}
	}

	//If the player has no colonies the ship goes directly onto the hyperspace gate
	//This assumes the player is the offense, which will be true 99% of the time, but technically Remora could have no colonies and retrieve a ship in response to the offense retrieving a ship.
	//It's not even clear from the rulebook what should happen in that case. Perhap's Remora's ability shouldn't resolve
	if(!valid_colonies.size())
	{
		std::cout << "The  " << to_string(p.color) << " player has no valid colonies! Placing the ship directly on the hyperspace gate.\n";
		hyperspace_gate.push_back(p.color);
	}
	else
	{
		std::stringstream prompt;
		prompt << "The " << to_string(p.color) << " player has the following valid colonies to choose from:\n";
		for(unsigned i=0; i<valid_colonies.size(); i++)
		{
			prompt << "Option " << i << ": " << to_string(valid_colonies[i].first) << " Planet " << valid_colonies[i].second << "\n";
		}
		unsigned chosen_option;
		do
		{
			std::cout << "Please choose one of the option numbers above.\n";
			std::cout << to_string(p.color) << ">>";
			std::cin >> chosen_option;
		} while(chosen_option >= valid_colonies.size()); //TODO: What's the proper way to protect bad input here? We don't want to crash, we just want to retry the prompt

		const std::pair<PlayerColors,unsigned> chosen_colony = valid_colonies[chosen_option];

		//Now actually add the colony
		bool colony_found = false; //paranoia
		for(auto player_begin=players.begin(),player_end=players.end();player_begin!=player_end;++player_begin)
		{
			if(player_begin->color != chosen_colony.first)
				continue;
			for(unsigned planet=0; planet<player_begin->planets.size(); planet++)
			{
				for(unsigned col=0; col<player_begin->planets[planet].size(); col++)
				{
					player_begin->planets[planet].push_back(p.color);
					colony_found = true;
					break;
				}
				if(colony_found)
					break;
			}
			if(colony_found)
				break;
		}

		assert(colony_found && "Failed to find colony to place ship!");
	}

	GameEvent g(p.color,GameEventType::RetrieveWarpShip);
	resolve_game_event(g);
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
	unsigned player_index = 6; //Sentinel value meant to be invalid
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
