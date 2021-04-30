#include <cstdlib>
#include <iostream>

#include "Oracle.hpp"
#include "GameState.hpp"

Oracle::Oracle()
{
	set_name("Oracle");
	set_power("Foresight");
	set_role(PlayerRole::MainPlayer);
	set_mandatory(true);
	valid_phases_insert(TurnPhase::Planning_before_selection); //Using before selection here to set up the turn more smoothly. Otherwise Oracle could pick a card before and after their opponent reveals their card, which is just awkward
	set_description("As a main player, before encounter cards are selected, use this power to force your opponent to play his or her encounter card faceup. Only after you have seen your opponent's card do you select and play your card.");
}

std::function<void()> Oracle::get_resolution_callback(GameState *g, const PlayerColors player, GameEvent &this_event, const GameEvent responding_to)
{
	std::function<void()> ret = [g,player] () { g->forsee_opponent_encounter_card(player); };
	return ret;
}

