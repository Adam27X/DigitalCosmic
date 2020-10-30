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
	std::map<PlayerColors,unsigned> offensive_allies; //Map ally color to the number of ships provided
	std::map<PlayerColors,unsigned> defensive_allies;
	const unsigned int max_player_sentinel = 6; //Sentintel value that's never a valid player ID
	CosmicCardType offensive_encounter_card;
	CosmicCardType defensive_encounter_card;
	PlayerColors player_receiving_compensation;
	PlayerColors player_giving_compensation;
	unsigned offense_attack_value;
	unsigned defense_attack_value;
	bool human_wins_encounter;
	bool stop_compensation_and_rewards;
	bool offense_won_encounter;

	void clear()
	{
		offense = PlayerColors::Invalid;
		defense = PlayerColors::Invalid;
		planet_location = PlayerColors::Invalid;
		planet_id = max_player_sentinel;
		offensive_allies.clear();
		defensive_allies.clear();
		offensive_encounter_card = CosmicCardType::None;
		defensive_encounter_card = CosmicCardType::None;
		player_receiving_compensation = PlayerColors::Invalid;
		player_giving_compensation = PlayerColors::Invalid;
		offense_attack_value = 0;
		defense_attack_value = 0;
		human_wins_encounter = false;
		stop_compensation_and_rewards = false;
		offense_won_encounter = false;
	}
};

class DealParameters
{
public:
	DealParameters() { clear(); }
	bool successful;
	unsigned num_cards_to_offense;
	bool cards_to_offense_chosen_randomly;
	unsigned num_cards_to_defense;
	bool cards_to_defense_chosen_randomly;
	bool offense_receives_colony;
	bool defense_receives_colony;
	void clear() { successful = false; num_cards_to_offense = 0; cards_to_offense_chosen_randomly = 0; num_cards_to_defense = 0; cards_to_defense_chosen_randomly = 0; offense_receives_colony = false; defense_receives_colony = false; }
};

class GameState
{
public:
	GameState(unsigned nplayers);
	void dump() const;
	void dump_planets() const;
	void dump_PlanetInfo(const PlanetInfo &source, const std::string name) const;
	void dump_destiny_deck() const;
	void dump_cosmic_deck() const;
	void assign_alien(const PlayerColors color, std::unique_ptr<AlienBase> &alien);
	void deal_starting_hands();
	void dump_player_hands() const;
	void dump_player_hand(const PlayerInfo &p) const;
	void choose_first_player();
	PlayerInfo& get_player(const PlayerColors &c);
	void discard_and_draw_new_hand(PlayerInfo &player);
	void resolve_game_event(const GameEvent g);
	std::string prompt_player(PlayerInfo &p, const std::string &prompt) const;
	void dump_current_stack() const;
	void draw_cosmic_card(PlayerInfo &player);
	void move_ship_to_colony(PlayerInfo &p, PlanetInfo &source, bool source_is_warp=false);
	void swap_encounter_cards(); //Sorcerer Alien power
	void swap_main_player_hands(); //Trader Alien power
	void add_reinforcements(const PlayerColors player, const unsigned value);
	void human_encounter_win_condition();
	void start_game();
	bool player_has_ship_in_warp(const PlayerColors player) const;
	void zap_alien(const PlayerColors player);

	void set_invalidate_next_callback(bool b) { invalidate_next_callback = b; }
	void add_to_discard_pile(const CosmicCardType c) { cosmic_discard.push_back(c); }
	PlanetInfo& get_warp() { return warp; }
	DealParameters& get_deal_params() { return deal_params; }

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
	std::vector< std::pair<PlayerColors,unsigned> > get_valid_colonies(const PlayerColors color) const;
	void cast_plague(const PlayerColors casting_player);
	void cast_force_field(const PlayerColors casting_player);
	void plague_player();
	void stop_allies();
	const std::pair<PlayerColors,unsigned> prompt_valid_colonies(const PlayerColors color, const std::vector< std::pair<PlayerColors,unsigned> > &valid_colonies);
	void choose_opponent_planet();
	void send_in_ships(const PlayerColors player);
	std::set<PlayerColors> invite_allies(const std::set<PlayerColors> &potential_allies, bool offense);
	void form_alliances(std::set<PlayerColors> &invited_by_offense, std::set<PlayerColors> &invited_by_defense);
	void lose_ships_to_warp(const PlayerColors player, const unsigned num_ships);
	void setup_negotiation();
	void resolve_negotiation();
	void setup_compensation();
	void resolve_compensation();
	void setup_attack();
	void resolve_attack();
	void offense_win_resolution();
	void defense_win_resolution();
	void force_negotiation();
	void resolve_human_encounter_win();
	void update_player_scores();
	void execute_turn();
	std::vector<PlayerColors> get_player_order();

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
	DealParameters deal_params;
	bool is_second_encounter_for_offense;
};

