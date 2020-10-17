#include <iostream>
#include <map>
#include <vector>
#include <cassert>
#include <algorithm>

#include "GameState.hpp"

std::string to_string(const PlayerColors &p)
{
	std::string ret;
	switch(p)
	{
		case PlayerColors::Red:
			ret = "Red";
		break;

		case PlayerColors::Blue:
			ret = "Blue";
		break;

		case PlayerColors::Purple:
			ret = "Purple";
		break;

		case PlayerColors::Yellow:
			ret = "Yellow";
		break;

		case PlayerColors::Green:
			ret = "Green";
		break;

		default:
			assert(0 && "Invalid player color!");
		break;
	}

	return ret;
}

void PlayerInfo::dump_hand() const
{
	std::cout << "Hand for the " << to_string(color) << " player:\n";
	for(auto i=hand.begin(),e=hand.end();i!=e;++i)
	{
		std::cout << to_string(*i) << "\n";
	}
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

	//If the new hand isn't valid, try again
	if(!player.has_encounter_cards_in_hand())
		discard_and_draw_new_hand(player);
}

void GameState::execute_turn(PlayerColors off)
{
	//Start Turn Phase
	state = TurnPhase::StartTurn;

	//Ensure the offense has a valid hand
	PlayerInfo &offense = get_player(off);
	bool offense_needs_discard = !offense.has_encounter_cards_in_hand();

	if(offense_needs_discard)
	{
		std::cout << "The offense has no encounter cards in hand. They now must discard their hand and draw eight cards\n";
		//discard_and_draw_new_hand(offense);
		//TODO: Support a stack and reactions to this event
		//std::function<void(PlayerInfo&)> f = [this](PlayerInfo &p) { discard_and_draw_new_hand(p); };
		std::function<void()> f = [this,&offense] () { discard_and_draw_new_hand(offense); };
		stack.push(f);
		check_for_resolution();
	}
}

void GameState::check_for_resolution()
{
	//TODO: Remora is an example of a response to discard_and_draw...how can we detect that Remora could respond? Perhaps in addition to or instead of adding functions we could add more specific actions such as 'player draw'
	//Basically we can have a stack of any actions that can be responded to
}
