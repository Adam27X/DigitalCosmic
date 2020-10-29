#pragma once

#include <set>
#include <string>
#include <cassert>

#include "GameEvent.hpp"
#include "TurnPhase.hpp"
#include "EncounterRole.hpp"
#include "PlayerRole.hpp"

class GameState;

class AlienBase
{
public:
	AlienBase() { };
	virtual void dump() const;

	void set_name(const std::string &n) { name = n; }
	void set_power(const std::string &p) { power = p; }
	void set_role(const PlayerRole &r) { role = r; }
	void set_mandatory(bool m) { mandatory = m; }
	bool get_mandatory() const { return mandatory; }
	void valid_phases_insert(const TurnPhase &t) { valid_phases.insert(t); }
	void set_description(const std::string &s) { description = s; }
	virtual bool can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const;
	virtual bool must_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const;
	//By default, Aliens cannot go on the stack (could change this to check for role/phase and have aliens that *can't* go on the stack override this function OR this function could be made pure virtual)
	virtual bool check_for_game_event(const EncounterRole e, const TurnPhase t) const { return false; }
	//Every Alien should uniquely define what happens when their power resolves
	virtual std::function<void()> get_resolution_callback(GameState *g, const PlayerColors player, const GameEvent ge) = 0;
	//By default, do nothing if countered (certain Aliens, like Human, will actually take an action if zapped)
	virtual std::function<void()> get_callback_if_countered(GameState *g, const PlayerColors player) { return nullptr; }
	const std::string& get_name() const { return name; }

private:
	std::string name;
	std::string power; //You have the power of...
	PlayerRole role; //Which role the player must have to use the alien power
	bool mandatory; //Is the Alien power optional or mandatory?
	std::set<TurnPhase> valid_phases; //When can/must the player exercise the power?
	std::string description; //Actual text from Alien card
};
