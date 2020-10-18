#include <cassert>
#include <iostream>
#include "PlayerRole.hpp"

std::string to_string(const PlayerRole &p)
{
	std::string ret;
	switch(p)
	{
		case PlayerRole::AnyPlayer:
			ret = "As any player";
		break;

		case PlayerRole::MainPlayer:
			ret = "Main player only";
		break;

		case PlayerRole::Offense:
			ret = "Offense only";
		break;

		case PlayerRole::MainPlayerOrAlly:
			ret = "Main player or ally only";
		break;

		case PlayerRole::NotMainPlayer:
			ret = "Not main player";
		break;

		case PlayerRole::NotOffense:
			ret = "Not offense";
		break;

		default:
			assert(0 && "Invalid Player role for Alien!");
		break;
	}

	return ret;
}

