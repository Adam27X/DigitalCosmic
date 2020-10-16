#pragma once

#include <vector>
#include <string>

enum class PlayerRole
{
	AnyPlayer,
	MainPlayer,
	Offense,
	MainPlayerOrAlly,
	NotMainPlayer,
	NotOffense
};

std::string to_string(const PlayerRole &p);

enum class TurnPhase
{
	StartTurn,
	Regroup,
	Destiny,
	Launch,
	Alliance,
	Planning,
	Reveal,
	Resolution
};

std::string to_string(const TurnPhase &t);

class AlienBase
{
public:
	AlienBase() { };
	virtual void dump() const;

	void set_name(const std::string &n) { name = n; }
	void set_power(const std::string &p) { power = p; }
	void set_role(const PlayerRole &r) { role = r; }
	void set_mandatory(bool m) { mandatory = m; }
	void valid_phases_push_back(const TurnPhase &t) { valid_phases.push_back(t); }

private:
	std::string name;
	std::string power; //You have the power of...
	PlayerRole role; //Which role the player must have to use the alien power
	bool mandatory; //Is the Alien power optional or mandatory?
	std::vector<TurnPhase> valid_phases; //When can/must the player exercise the power?
};
