#include <cstdlib>
#include <iostream>

#include "Sorcerer.hpp"
#include "GameState.hpp"

Sorcerer::Sorcerer()
{
	set_name("Sorcerer");
	set_power("Magic");
	set_role(PlayerRole::MainPlayer);
	set_mandatory(false);
	valid_phases_insert(TurnPhase::Planning_after_selection);
	set_description("As a main player, after encounter cards are selected, but before they are revealed, you *may use* this power to switch encounter cards with your opponent so that he or she reveals your card and you reveal your opponent's card.");
}

bool Sorcerer::check_for_game_event(const EncounterRole e, const TurnPhase t) const
{
	//TODO: What's the point of setting PlayerRole here if we're doing checks like this?
	//We could write some other function that deals with EncounterRole and PlayerRole reconciliation
	//Same goes for valid_phases...in fact, could this logic be pushed down to AlienBase?
	if((e == EncounterRole::Offense || e == EncounterRole::Defense) && t == TurnPhase::Planning_after_selection)
	{
		return true;
	}

	return false;
}

std::function<void()> Sorcerer::get_resolution_callback(GameState *g, const PlayerColors player, const GameEvent ge)
{
	std::function<void()> ret = [g] () { g->swap_encounter_cards(); };
	return ret;
}

