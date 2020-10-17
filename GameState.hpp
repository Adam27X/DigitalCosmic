#pragma once

#include <vector>
#include <memory>
#include <stack>
#include <functional>

#include "DestinyDeck.hpp"
#include "CosmicDeck.hpp"
#include "AlienBase.hpp"
#include "PlayerInfo.hpp"
#include "GameEvent.hpp"

class GameState
{
public:
	GameState(unsigned nplayers);
	void dump() const;
	void dump_destiny_deck() const;
	void dump_cosmic_deck() const;
	void assign_alien(const PlayerColors color, std::unique_ptr<AlienBase> &alien);
	void deal_starting_hands();
	void dump_player_hands() const;
	void dump_player_hand(const PlayerInfo &p) const;
	PlayerColors choose_first_player();
	void execute_turn(PlayerColors offense);
	PlayerInfo& get_player(const PlayerColors &c);
	void discard_and_draw_new_hand(PlayerInfo &player);
	void resolve_game_event(const GameEvent g);
	std::string prompt_player(PlayerInfo &p, const std::string &prompt) const;
	void dump_current_stack() const;
	void draw_cosmic_card(PlayerInfo &player);
	void set_invalidate_next_callback(bool b) { invalidate_next_callback = b; }
private:	
	void shuffle_destiny_deck();
	void shuffle_cosmic_deck();
	void shuffle_discard_into_cosmic_deck();
	unsigned num_players;
	std::vector<PlayerInfo> players;
	DestinyDeck destiny_deck;
	CosmicDeck cosmic_deck;
	std::vector<CosmicCardType> cosmic_discard;
	//std::stack< std::function<void()> > stack; //Stack of events?
	std::stack<GameEvent> stack;
	TurnPhase state;
	bool invalidate_next_callback;
};

