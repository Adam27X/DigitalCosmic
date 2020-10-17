#include <cassert>
#include "GameEvent.hpp"

std::string to_string(const GameEventType &g)
{
	std::string ret;
	switch(g)
	{
		case GameEventType::DrawCard:
			ret = "Draw Card";
		break;

		case GameEventType::AlienPower:
			ret = "Alien Power";
		break;

		case GameEventType::CosmicZap:
			ret = "Cosmic Zap";
		break;

		case GameEventType::None:
			assert(0 && "Tried to print string of GameEventType::None, which is an invalid GameEventType");
		break;

		default:
			assert(0 && "Unknown GameEvent type!\n");
		break;
	}

	return ret;
}
