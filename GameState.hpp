#pragma once

#include <vector>
#include <memory>
#include <stack>
#include <functional>
#include <map>
#include <set>

#include "DestinyDeck.hpp"
#include "CosmicDeck.hpp"
#include "AlienBase.hpp"
#include "PlayerInfo.hpp"
#include "GameEvent.hpp"

class PlayerAssignments
{
public:
	PlayerAssignments() { clear(); }
	PlayerColors offense;
	PlayerColors defense;
	PlayerColors planet_location;
	unsigned planet_id;
	std::map<PlayerColors,unsigned> offensive_allies;
	std::map<PlayerColors,unsigned> defensive_allies;
	const unsigned int max_player_sentinel = 6; //Sentintel value that's never a valid player ID
	CosmicCardType offensive_encounter_card;
	CosmicCardType defensive_encounter_card;
	void clear() { offense = PlayerColors::Invalid; defense = PlayerColors::Invalid; planet_location = PlayerColors::Invalid; planet_id = max_player_sentinel; offensive_allies.clear(); defensive_allies.clear(); offensive_encounter_card = CosmicCardType::None; defensive_encounter_card = CosmicCardType::None; }
};

class GameState
{
public:
	GameState(unsigned nplayers);
	void dump() const;
	void dump_planets() const;
	void dump_hyperspace_gate() const;
	void dump_destiny_deck() const;
	void dump_cosmic_deck() const;
	void assign_alien(const PlayerColors color, std::unique_ptr<AlienBase> &alien);
	void deal_starting_hands();
	void dump_player_hands() const;
	void dump_player_hand(const PlayerInfo &p) const;
	void choose_first_player();
	void execute_turn();
	PlayerInfo& get_player(const PlayerColors &c);
	void discard_and_draw_new_hand(PlayerInfo &player);
	void resolve_game_event(const GameEvent g);
	std::string prompt_player(PlayerInfo &p, const std::string &prompt) const;
	void dump_current_stack() const;
	void draw_cosmic_card(PlayerInfo &player);
	void move_ship_to_colony(PlayerInfo &p, PlanetInfo &source);
	void swap_encounter_cards(); //Sorcerer Alien power
	void swap_main_player_hands(); //Trader Alien power

	void set_invalidate_next_callback(bool b) { invalidate_next_callback = b; }
	void add_to_discard_pile(const CosmicCardType c) { cosmic_discard.push_back(c); }
	PlanetInfo& get_warp() { return warp; }

	//Methods only meant for testing
	void debug_send_ship_to_warp();
private:	
	void shuffle_destiny_deck();
	void draw_from_destiny_deck();
	void shuffle_cosmic_deck();
	void shuffle_discard_into_cosmic_deck();
	void free_all_ships_from_warp();
	void get_callbacks_for_cosmic_card(const CosmicCardType play, GameEvent &g);
	void check_for_game_events(PlayerInfo &offense);
	std::vector< std::pair<PlayerColors,unsigned> > get_valid_colonies(const PlayerColors color);
	void cast_plague(const PlayerColors casting_player);
	void cast_force_field(const PlayerColors casting_player);
	void plague_player();
	void stop_allies();
	const std::pair<PlayerColors,unsigned> prompt_valid_colonies(const PlayerColors color, const std::vector< std::pair<PlayerColors,unsigned> > &valid_colonies);
	void choose_opponent_planet();
	void send_in_ships(const PlayerColors player);
	std::set<PlayerColors> invite_allies(const std::set<PlayerColors> &potential_allies, bool offense);
	void form_alliances(std::set<PlayerColors> &invited_by_offense, std::set<PlayerColors> &invited_by_defense);

	unsigned num_players;
	std::vector<PlayerInfo> players;
	DestinyDeck destiny_deck;
	CosmicDeck cosmic_deck;
	std::vector<CosmicCardType> cosmic_discard;
	std::stack<GameEvent> stack;
	TurnPhase state;
	bool invalidate_next_callback;
	const unsigned int max_player_sentinel = 6; //Sentintel value that's never a valid player ID
	unsigned player_to_be_plagued;
	PlanetInfo warp; //The warp operates similarly to a planet
	PlanetInfo hyperspace_gate; //As does the hyperspace gate
	PlanetInfo defensive_ally_ships;
	PlayerAssignments assignments;
	std::set<PlayerColors> allies_to_be_stopped; //Allies to be prevented by the player casting force field (if it resolves)
};

