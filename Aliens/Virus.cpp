#include <cstdlib>
#include <iostream>

#include "Virus.hpp"
#include "GameState.hpp"

Virus::Virus()
{
	set_name("Virus");
	set_power("Multiplication");
	set_role(PlayerRole::MainPlayer);
	set_mandatory(true);
	valid_phases_insert(TurnPhase::Reveal);
	set_description("As a main player, after you reveal an attack card in an encounter, use this power to multiply the number of ships you have in the encounter times the value of your card instead of adding. Your allies' ships are still added to your total as usual.");
}

std::function<void()> Virus::get_resolution_callback(GameState *g, const PlayerColors player, GameEvent &this_event, const GameEvent responding_to)
{
	std::function<void()> ret = [g,player] () { g->evaluate_encounter_cards(player); };
	return ret;
}

