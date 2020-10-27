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

		case GameEventType::CardZap:
			ret = "Card Zap";
		break;

		case GameEventType::RetrieveWarpShip:
			ret = "Retrieve Warp Ship";
		break;

		case GameEventType::MobiusTubes:
			ret = "Mobius Tubes";
		break;

		case GameEventType::Plague:
			ret = "Plague";
		break;

		case GameEventType::EmotionControl:
			ret = "Emotion Control";
		break;

		case GameEventType::ForceField:
			ret = "Force Field";
		break;

		case GameEventType::Reinforcement2:
			ret = "Reinforcement +2";
		break;

		case GameEventType::Reinforcement3:
			ret = "Reinforcement +3";
		break;

		case GameEventType::Reinforcement5:
			ret = "Reinforcement +5";
		break;

		case GameEventType::Quash:
			ret = "Quash";
		break;

		case GameEventType::IonicGas:
			ret = "Ionic Gas";
		break;

		case GameEventType::SuccessfulNegotiation:
			ret = "Successful Negotiation";
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
