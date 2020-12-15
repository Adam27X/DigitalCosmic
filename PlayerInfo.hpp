#pragma once

#include <vector>
#include <memory>

#include "CosmicDeck.hpp"
#include "PlayerColors.hpp"
#include "AlienBase.hpp"

//TODO: Use PlanetInfoFull<Playercolors> here too?
typedef std::vector<PlayerColors> PlanetInfo;

class GameState;

//Represents a player's set of planets
class PlanetSystem
{
public:
	void resize(unsigned size) { planets.resize(size); }
	unsigned size() const { return planets.size(); }
	std::vector<PlanetInfo>::iterator begin() { return planets.begin(); }
	std::vector<PlanetInfo>::iterator end() { return planets.end(); }
	std::vector<PlanetInfo>::const_iterator cbegin() const { return planets.cbegin(); }
	std::vector<PlanetInfo>::const_iterator cend() const { return planets.cend(); }
	PlanetInfo& get_planet(unsigned planet_index) { return planets[planet_index]; }

	unsigned planet_size(unsigned planet_index) const { return planets[planet_index].size(); }
	PlayerColors get_ship(unsigned planet_index, unsigned ship_index) const { return planets[planet_index][ship_index]; }
	PlanetInfo::iterator planet_begin(unsigned planet_index) { return planets[planet_index].begin(); }
	PlanetInfo::iterator planet_end(unsigned planet_index) { return planets[planet_index].end(); }
	template<typename Iterator>
	Iterator planet_erase(unsigned planet_index, Iterator pos)
	{
		auto ret = planets[planet_index].erase(pos);
		planet_update_function();
		return ret;

	}
	void planet_push_back(unsigned planet_index, const PlayerColors c)
	{
		planets[planet_index].push_back(c);
		planet_update_function();
	}

	void set_server_callback(std::function<void()> f) { planet_update_function = f; }

private:
	std::vector<PlanetInfo> planets;
	std::function<void()> planet_update_function;
};

class PlayerInfo
{
public:
	PlayerColors color;
	unsigned score;
	PlanetSystem planets;
	std::unique_ptr<AlienBase> alien;
	EncounterRole current_role;
	bool alien_zapped;

	void make_default_player(const PlayerColors c);
	void dump_hand() const;
	std::string get_hand() const;
	bool has_encounter_cards_in_hand() const;
	std::vector<GameEvent> can_respond(TurnPhase t, GameEvent g);
	void set_game_state(GameState *g);
	GameEvent can_use_alien_with_empty_stack(const TurnPhase t);
	bool alien_enabled() const;
	const std::string get_alien_desc() const;
	bool alien_revealed() const;
	void discard_card_callback_helper(const CosmicCardType c) const;

	template<typename T>
	std::function<void()> discard_card_callback(T i)
	{
		return [this,i] () { this->discard_card_callback_helper(*i); this->hand_erase(i); };
	}

	void update_client_hand() const;
	void hand_push_back(const CosmicCardType c);
	std::vector<CosmicCardType>::iterator hand_begin() { return hand.begin(); }
	std::vector<CosmicCardType>::iterator hand_end() { return hand.end(); }
	void hand_clear();
	unsigned hand_size() const { return hand.size(); }
	bool hand_empty() const { return hand.empty(); }
	CosmicCardType hand_get(unsigned index) { return hand[index]; }
	std::vector<CosmicCardType> get_hand_data() const { return hand; }
	void set_hand_data(std::vector<CosmicCardType> data) { hand = data; }

	template<typename Iterator>
	Iterator hand_erase(Iterator pos)
	{
		auto ret = hand.erase(pos);
	        update_client_hand();
		return ret;
	}

	template<typename Iterator>
	Iterator hand_erase(Iterator first, Iterator last)
	{
		auto ret = hand.erase(first,last);
	        update_client_hand();
		return ret;
	}

	GameState *game; //Intended for callbacks and should be used sparingly

private:
	std::vector<CosmicCardType> hand;

};

