#include <cstdlib>
#include <iostream>

#include "Warpish.hpp"
#include "GameState.hpp"

Warpish::Warpish()
{
	set_name("Warpish");
	set_power("Necromancy");
	set_role(PlayerRole::MainPlayer);
	set_mandatory(true);
	valid_phases_insert(TurnPhase::Reveal);
	set_description("As a main player, after you reveal an attack card in an encounter, use this power to add 1 to your total for each ship (yours or otherwise) in the warp.");
}

std::function<void()> Warpish::get_resolution_callback(GameState *g, const PlayerColors player, GameEvent &this_event, const GameEvent responding_to)
{
	std::function<void()> ret = [g,player,this_event] () { g->apply_necromancy(player); };
	return ret;
}

