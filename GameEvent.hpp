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
	DefensiveEncounterLoss,
	CosmicDeckShuffle,
	EncounterWin,
	CastFlare,
	NewColony,
	CrashLandTrigger,
	CrashLandSuperTrigger,
	DestinyCardDrawn,
	DestinyWildDrawn,
	//NOTE: Flares should be last here (other than none) for ease of grouping them together in other logic
	Flare_TickTock_Wild,
	Flare_TickTock_Super,
	Flare_Human_Wild,
	Flare_Human_Super,
	Flare_Remora_Wild,
	Flare_Remora_Super,
	Flare_Trader_Wild,
	Flare_Trader_Super,
	Flare_Sorcerer_Wild,
	Flare_Sorcerer_Super,
	Flare_Virus_Wild,
	Flare_Virus_Super,
	Flare_Spiff_Wild,
	Flare_Spiff_Super,
	Flare_Machine_Wild,
	Flare_Machine_Super,
	None
};

std::string to_string(const GameEventType &g);
bool is_flare(const GameEventType g);

class GameEvent
{
public:
	GameEvent(PlayerColors c, GameEventType g) : player(c), event_type(g), reinforcements_to_offense(false) { }

	PlayerColors player;
	GameEventType event_type;
	std::function<void()> callback_if_resolved; //Action to perform once the event resolves
	std::function<void()> callback_if_action_taken; //Happens regardless if the event resolves
	std::function<void()> callback_if_countered; //Rarely needed, but one case is for the Human; if the Human is zapped then their side wins the encounter
	std::string aux; //Any auxiliary information that goes with this event; Example: the player targeted by a plague

	//Fields relevant to specific events
	bool reinforcements_to_offense; //For reinforcement cards, will the reinforcements be added to the offense or defense?
};

