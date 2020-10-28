#pragma once

#include <string>
#include <functional>
#include "PlayerColors.hpp"

//Game actions that go on a stack and can be responded to
enum class GameEventType
{
	DrawCard,
	AlienPower,
	CosmicZap,
	CardZap,
	RetrieveWarpShip,
	MobiusTubes,
	Plague,
	EmotionControl,
	ForceField,
	Reinforcement2,
	Reinforcement3,
	Reinforcement5,
	Quash,
	IonicGas,
	SuccessfulNegotiation, //Negotiation has not yet resolved and can still be quashed
	SuccessfulDeal, //Negotiation actually resolved
	DefensiveEncounterWin,
	None
};

std::string to_string(const GameEventType &g);

class GameEvent
{
public:
	GameEvent(PlayerColors c, GameEventType g) : player(c), event_type(g) { }

	PlayerColors player;
	GameEventType event_type;
	std::function<void()> callback_if_resolved; //Action to perform once the event resolves
	std::function<void()> callback_if_action_taken; //Happens regardless if the event resolves
	std::function<void()> callback_if_countered; //Rarely needed, but one case is for the Human; if the Human is zapped then their side wins the encounter
};

