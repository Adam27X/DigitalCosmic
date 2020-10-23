#pragma once

#include <vector>
#include <string>

#include "GameEvent.hpp"
#include "TurnPhase.hpp"
#include "EncounterRole.hpp"
#include "PlayerRole.hpp"

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
	void valid_phases_push_back(const TurnPhase &t) { valid_phases.push_back(t); }
	void set_description(const std::string &s) { description = s; }
	virtual bool can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const;
	virtual bool must_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const;
	//TODO: Make this a pure virtual function and make AlienBase an abstract base class?
	virtual bool check_for_game_event(const EncounterRole e, const TurnPhase t) const { return false; }

private:
	std::string name;
	std::string power; //You have the power of...
	PlayerRole role; //Which role the player must have to use the alien power
	bool mandatory; //Is the Alien power optional or mandatory?
	//FIXME: This should be a std::set
	std::vector<TurnPhase> valid_phases; //When can/must the player exercise the power?
	std::string description; //Actual text from Alien card
};
