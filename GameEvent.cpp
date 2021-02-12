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
			ret = "Successful Negotiation (not yet resolved)";
		break;

		case GameEventType::SuccessfulDeal:
			ret = "Successful Negotiation resolved";
		break;

		case GameEventType::DefensiveEncounterWin:
			ret = "Defense won the encounter";
		break;

		case GameEventType::CosmicDeckShuffle:
			ret = "Shuffling of the Cosmic Deck";
		break;

		case GameEventType::Flare_TickTock_Wild:
			ret = "Flare: Tick-Tick (Wild)";
		break;

		case GameEventType::Flare_TickTock_Super:
			ret = "Flare: Tick-Tock (Super)";
		break;

		case GameEventType::Flare_Human_Wild:
			ret = "Flare: Human (Wild)";
		break;

		case GameEventType::Flare_Human_Super:
			ret = "Flare: Human (Super)";
		break;

		case GameEventType::Flare_Remora_Wild:
			ret = "Flare: Remora (Wild)";
		break;

		case GameEventType::Flare_Remora_Super:
			ret = "Flare: Remora (Super)";
		break;

		case GameEventType::Flare_Trader_Wild:
			ret = "Flare: Trader (Wild)";
		break;

		case GameEventType::Flare_Trader_Super:
			ret = "Flare: Trader (Super)";
		break;

		case GameEventType::Flare_Sorcerer_Wild:
			ret = "Flare: Sorcerer (Wild)";
		break;

		case GameEventType::Flare_Sorcerer_Super:
			ret = "Flare: Sorcerer (Super)";
		break;

		case GameEventType::EncounterWin:
			ret = "A player won an encounter";
		break;

		case GameEventType::CastFlare:
			ret = "A flare was casted";
		break;

		case GameEventType::NewColony:
			ret = "A new colony was established";
		break;

		case GameEventType::CrashLandTrigger:
			ret = "Offense lost the encounter by 10 or more";
		break;

		case GameEventType::DestinyCardDrawn:
			ret = "A (non wild) card was drawn from the destiny deck";
		break;

		case GameEventType::DestinyWildDrawn:
			ret = "A Wild card was drawn from the destiny deck";
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

bool is_flare(const GameEventType g)
{
	return g >= GameEventType::Flare_TickTock_Wild && g < GameEventType::None;
}

