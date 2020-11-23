#pragma once

#include <vector>
#include <memory>

#include "CosmicDeck.hpp"
#include "PlayerColors.hpp"
#include "AlienBase.hpp"

typedef std::vector<PlayerColors> PlanetInfo;

class GameState;

class PlayerInfo
{
public:
	PlayerColors color;
	unsigned score;
	std::vector<PlanetInfo> planets; //Each planet has some number of ships of each valid player color
	std::unique_ptr<AlienBase> alien;
	EncounterRole current_role;
	bool alien_zapped;

	void make_default_player(const PlayerColors c);
	void dump_hand() const;
	std::string get_hand() const;
	bool has_encounter_cards_in_hand() const;
	std::vector<GameEvent> can_respond(TurnPhase t, GameEvent g);
	void set_game_state(GameState *g) { game = g; }
	GameEvent can_use_alien_with_empty_stack(const TurnPhase t);
	bool alien_enabled() const;
	const std::string get_alien_desc() const;
	bool alien_revealed() const;

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

	//Does this need a forward decl?
	GameState *game; //Intended for callbacks and should be used sparingly

private:
	std::vector<CosmicCardType> hand;

};

