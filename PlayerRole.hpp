#pragma once

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

