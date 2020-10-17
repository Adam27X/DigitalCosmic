#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>

#include "GameState.hpp"

PlayerInfo GameState::make_default_player(const PlayerColors color)
{
	PlayerInfo player;
	player.score = 0;

	player.color = color;
	const unsigned num_planets_per_player = 5;
	const unsigned default_ships_per_planet = 4;
	player.planets.resize(num_planets_per_player);
	for(unsigned i=0; i<player.planets.size(); i++)
	{
		player.planets[i].push_back(std::make_pair(player.color,default_ships_per_planet));
	}

	return player;
}

GameState::GameState(unsigned nplayers) : num_players(nplayers), destiny_deck(DestinyDeck(nplayers))
{
	assert(nplayers > 1 && nplayers < 6 && "Invalid number of players!");
	std::cout << "Starting Game with " << num_players << " players\n";

	//For now assign colors in a specific order...TODO: let the user choose colors
	players.resize(num_players);
	players[0] = make_default_player(PlayerColors::Red);
	players[1] = make_default_player(PlayerColors::Blue);
	if(num_players > 2)
	{
		players[2] = make_default_player(PlayerColors::Purple);
	}
	if(num_players > 3)
	{
		players[3] = make_default_player(PlayerColors::Yellow);
	}
	if(num_players > 4)
	{
		players[4] = make_default_player(PlayerColors::Green);
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
				std::cout << iii->second << "(" << to_string(iii->first) << ")";
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

void GameState::dump_player_hands() const
{
	for(auto i=players.begin(),e=players.end();i!=e;++i)
	{
		i->dump_hand();
	}
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
		//std::move(cosmic_discard.begin(),cosmic_discard.end(),cosmic_deck.begin());
		for(auto i=cosmic_discard.begin(),e=cosmic_discard.end();i!=e;++i)
		{
			cosmic_deck.push_back(*i);
		}
		cosmic_discard.clear();
		cosmic_deck.shuffle();
	}

	while(player.hand.size() < 8)
	{
		player.hand.push_back(*cosmic_deck.begin());
		cosmic_deck.erase(cosmic_deck.begin());
	}

	GameEvent g(player.color,GameEventType::DrawCard);
	stack.push(g);
	resolve_stack();

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
	bool offense_needs_discard = offense.has_encounter_cards_in_hand();

	std::cout << "The " << to_string(off) << " Player is now the offense.\n";
	if(offense_needs_discard)
	{
		std::cout << "The offense has no encounter cards in hand. They now must discard their hand and draw eight cards\n";
		discard_and_draw_new_hand(offense);
		//NOTE: This function always resolves so we just call it now; for more general game state actions that may or may not resolve, we can wrap them into std::function objects like below and put them into a stack to be processed
		//std::function<void(PlayerInfo&)> f = [this](PlayerInfo &p) { discard_and_draw_new_hand(p); };
		//std::function<void()> f = [this,&offense] () { discard_and_draw_new_hand(offense); };
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
		unsigned depth = stack.size()-1;
		std::cout << depth << ": " << to_string(g.player) << " -> " << to_string(g.event_type) << "\n";
		copy_stack.pop();
	}

	assert(copy_stack.empty() && "Error printing stack!");
}

void GameState::resolve_stack()
{
	//Peek at the top of the stack and ask players in turn order if they would like to respond or resolve (but only ask if they have a valid response)
	dump_current_stack();
	while(!stack.empty())
	{
		GameEvent g = stack.top();
		if(g.event_type == GameEventType::DrawCard)
		{
			//Only Remora can react to this event for now...Cosmic cards cannot (except possibly for some flares)
			//Iterate through players and if they have Remora as their Alien, ask if they would like to respond
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
				bool can_respond = players[player_index].can_respond(state,g);
				if(can_respond)
				{
					bool take_action = false;
					bool must_respond = players[player_index].must_respond(state,g);
					if(must_respond)
					{
						std::cout << "The " << to_string(players[player_index].color) << " player *must* respond to the draw action.\n";
						take_action = true;
					}
					else
					{
						std::cout << "The " << to_string(players[player_index].color) << " player can respond to the draw action.\n";
						std::string response;
					       	do
						{
							response = prompt_player(players[player_index],"Would you like to respond to the card draw with your Alien power? y/n\n");
						} while(response.compare("y") != 0 && response.compare("n") != 0);
						if(response.compare("y") == 0)
						{
							take_action = true;
						}
					}

					if(take_action)
					{
						//TODO:
					}
				}

				player_index = (player_index+1) % players.size();
			} while(player_index != initial_player_index);
			stack.pop();
		}
	}
}
