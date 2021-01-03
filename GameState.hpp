#pragma once

#include <vector>
#include <memory>
#include <stack>
#include <functional>
#include <map>
#include <set>
#include <algorithm>

#include "DestinyDeck.hpp"
#include "CosmicDeck.hpp"
#include "AlienBase.hpp"
#include "PlayerInfo.hpp"
#include "GameEvent.hpp"
#include "CosmicServer.hpp"
#include "PlanetInfo.hpp"

bool is_only_digits(const std::string &s);

class PlayerAssignments
{
public:
	PlayerAssignments(std::function<void()> off_callback, std::function<void()> def_callback) : offense_callback(off_callback), defense_callback(def_callback)
	{
		clear();
	}

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
	bool successful_encounter;
	bool reestablished_colony;

	void set_offense(const PlayerColors c)
	{
		offense = c;
		offense_callback();
	}
	const PlayerColors get_offense() const { return offense; }

	void set_defense(const PlayerColors c)
	{
		defense = c;
		defense_callback();
	}
	const PlayerColors get_defense() const { return defense; }

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
		successful_encounter = false;
		reestablished_colony = false;
	}

private:
	PlayerColors offense;
	PlayerColors defense;
	std::function<void()> offense_callback;
	std::function<void()> defense_callback;
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
	std::pair<PlayerColors,unsigned> colony_for_offense;
	std::pair<PlayerColors,unsigned> colony_for_defense;
	void clear()
	{
		successful = false;
		num_cards_to_offense = 0;
		cards_to_offense_chosen_randomly = 0;
		num_cards_to_defense = 0;
		cards_to_defense_chosen_randomly = 0;
		offense_receives_colony = false;
		defense_receives_colony = false;
		colony_for_offense = std::make_pair(PlayerColors::Invalid,0);
		colony_for_defense = std::make_pair(PlayerColors::Invalid,0);
	}
};

class GameState
{
public:
	GameState(unsigned nplayers, CosmicServer &serv);
	void dump() const;
	void dump_planets() const;
	void dump_warp() const;
	void dump_destiny_deck() const;
	void dump_cosmic_deck() const;
	void assign_alien(const PlayerColors color, std::unique_ptr<AlienBase> &alien);
	void deal_starting_hands();
	void dump_player_hands() const;
	void dump_player_hand(const PlayerInfo &p) const;
	void send_player_hands() const;
	void send_player_hand(const PlayerColors player) const;
	void broadcast_player_hand_size(const PlayerColors player) const;
	void choose_first_player();
	PlayerInfo& get_player(const PlayerColors &c);
	const PlayerInfo& get_player_const(const PlayerColors &c) const;
	void discard_and_draw_new_hand(PlayerInfo &player);
	void resolve_game_event(const GameEvent g);
	unsigned prompt_player(const PlayerColors player, const std::string &prompt, const std::vector<std::string> &options) const;
	void dump_current_stack() const;
	void draw_cosmic_card(PlayerInfo &player);
	void move_ship_to_colony(PlayerInfo &p, PlanetInfoVector<PlayerColors> &source);
	void move_ship_from_warp_to_colony(PlayerInfo &p);
	void swap_encounter_cards(); //Sorcerer Alien power
	void swap_main_player_hands(); //Trader Alien power
	void setup_reinforcements(GameEvent &g);
	void add_reinforcements(const GameEvent &g, const unsigned value, bool can_choose_side=true);
	void human_encounter_win_condition();
	void start_game();
	bool player_has_ship_in_warp_from_prior_encounter(const PlayerColors player) const;
	void zap_alien(const PlayerColors player);
	const std::string get_cosmic_discard() const;
	const std::string get_destiny_discard() const;
	const std::string get_planets() const;
	const std::string get_PlanetInfo(const PlanetInfoVector<PlayerColors> &source, const std::string name) const;
	const std::string get_warp_str() const;
	const std::string get_game_board() const;
	const CosmicServer& get_server() const { return server; }
	void update_planets() const;
	void update_tick_tock_tokens(unsigned tokens) const;
	void free_all_ships_from_warp();

	void set_invalidate_next_callback(bool b) { invalidate_next_callback = b; }
	void add_to_discard_pile(const CosmicCardType c) { cosmic_discard.push_back(c); }
	DealParameters& get_deal_params() { return deal_params; }

private:	
	void shuffle_destiny_deck();
	void draw_from_destiny_deck();
	void shuffle_cosmic_deck();
	void shuffle_discard_into_cosmic_deck();
	void get_callbacks_for_cosmic_card(const CosmicCardType play, GameEvent &g);
	void check_for_game_events();
	void check_for_game_events_helper(std::set<PlayerColors> &used_aliens_this_phase);
	std::vector< std::pair<PlayerColors,unsigned> > get_valid_colonies(const PlayerColors color) const;
	void cast_plague(const PlayerColors casting_player);
	void cast_force_field(const PlayerColors casting_player);
	void plague_player();
	void stop_allies();
	const std::pair<PlayerColors,unsigned> prompt_valid_colonies(const PlayerColors color, const std::vector< std::pair<PlayerColors,unsigned> > &valid_colonies, bool allow_none = false);
	void choose_opponent_planet();
	void send_in_ships(const PlayerColors player, bool custom_destination = false, const std::pair<PlayerColors,unsigned> dest_planet = std::make_pair(PlayerColors::Invalid,0));
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
	void update_turn_phase(const TurnPhase phase);
	void update_warp() const;
	void update_hyperspace_gate() const;
	void update_defensive_ally_ships() const;
	void update_offense() const;
	void update_defense() const;
	void update_cosmic_discard() const;
	void update_destiny_discard() const;
	void add_score_from_ships();
	void broadcast_encounter_scores() const;
	void end_of_turn_clean_up();

	unsigned num_players;
	std::vector<PlayerInfo> players;
	DestinyDeck destiny_deck;
	CosmicDeck cosmic_deck;
	std::stack<GameEvent> stack;
	TurnPhase state;
	bool invalidate_next_callback;
	const unsigned int max_player_sentinel = 6; //Sentintel value that's never a valid player ID
	unsigned player_to_be_plagued;
	std::set<PlayerColors> allies_to_be_stopped; //Allies to be prevented by the player casting force field (if it resolves)
	DealParameters deal_params;
	bool is_second_encounter_for_offense;
	unsigned encounter_num;
	CosmicServer &server;
	PlanetInfoVector< std::pair<PlayerColors,unsigned> > warp; //Vector of (ship,timestamp) pairs. We will rarely care about the timestamp
	PlanetInfoVector<PlayerColors> hyperspace_gate;
	PlanetInfoVector<PlayerColors> defensive_ally_ships;
	PlayerAssignments assignments;
	PlanetInfoVector<CosmicCardType> cosmic_discard;
};

