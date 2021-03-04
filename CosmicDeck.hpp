#pragma once

#include <vector>
#include "TurnPhase.hpp"
#include "EncounterRole.hpp"
#include "GameEvent.hpp"

enum class CosmicCardType
{
	Attack0 = 0, //1
	Attack1 = 1, //1
	Attack4 = 4, //4
	Attack5 = 5, //1
	Attack6 = 6, //7
	Attack7 = 7, //1
	Attack8 = 8, //7
	Attack9 = 9, //1
	Attack10 = 10, //4
	Attack11 = 11, //1
	Attack12 = 12, //2
	Attack13 = 13, //1
	Attack14 = 14, //2
	Attack15 = 15, //1
	Attack20 = 20, //2
	Attack23 = 23, //1
	Attack30 = 30, //1
	Attack40 = 40, //1
	Attack42 = 42, //0 (Not a real card, but used by the Wild Human flare)
	Negotiate, //15
	Morph, //1
	Reinforcement2, //2
	Reinforcement3, //3
	Reinforcement5, //1
	CardZap, //2
	CosmicZap, //2
	MobiusTubes, //2
	EmotionControl, //1
	ForceField, //1
	IonicGas, //1
	Plague, //1
	Quash, //1
	Flare_TickTock,
	Flare_Human,
	Flare_Remora,
	Flare_Trader,
	Flare_Sorcerer,
	Flare_Virus,
	Flare_Spiff,
	None
};

std::string to_string(const CosmicCardType &c);
//TODO: Restructure this function so that it takes in a const GameState reference or some other data structure instead of these random fields
bool can_play_card_with_empty_stack(const TurnPhase state, const CosmicCardType c, const EncounterRole role, bool alien_enabled, const std::string &alien_name, const std::string &opponent_alien_name, bool &super_flare, const std::string &offense_alien_name, const std::string &defense_alien_name);
GameEventType to_game_event_type(const CosmicCardType c, bool super_flare=false);
CosmicCardType to_cosmic_card_type(const GameEventType g);
bool is_flare(const CosmicCardType c);
bool is_attack_card(const CosmicCardType c);

class CosmicDeck
{
public:
	//TODO: Support Flares
	CosmicDeck();
	void shuffle();
	void dump() const;
	std::vector<CosmicCardType>::iterator begin() { return deck.begin(); }
	std::vector<CosmicCardType>::iterator end() { return deck.end(); }
	std::vector<CosmicCardType>::iterator erase(std::vector<CosmicCardType>::const_iterator position) { return deck.erase(position); }
	void clear() { deck.clear(); }
	unsigned size() const { return deck.size(); }
	bool empty() const { return deck.empty(); }
	void push_back(const CosmicCardType &c) { deck.push_back(c); }
private:
	std::vector<CosmicCardType> deck;
};

