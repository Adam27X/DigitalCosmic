#include <cstdlib>
#include <iostream>

#include "Trader.hpp"
#include "GameState.hpp"

Trader::Trader()
{
	set_name("Trader");
	set_power("Transference");
	set_role( PlayerRole::MainPlayer);
	set_mandatory(false);
	valid_phases_insert(TurnPhase::Planning_before_selection);
	//Use std::swap for the trade?
	set_description("As a main player, before encounter cards are selected, you *may use* this power to trade hands with your opponent. You each then keep the new hand.");
}

bool Trader::check_for_game_event(const EncounterRole e, const TurnPhase t) const
{
	if((e == EncounterRole::Offense || e == EncounterRole::Defense) && t == TurnPhase::Planning_before_selection)
	{
		return true;
	}

	return false;
}

std::function<void()> Trader::get_resolution_callback(GameState *g, const PlayerColors player, const GameEvent ge)
{
	std::function<void()> ret = [g] () { g->swap_main_player_hands(); };
	return ret;
}
