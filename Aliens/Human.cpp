#include <cstdlib>
#include <iostream>

#include "Human.hpp"
#include "GameState.hpp"

Human::Human()
{
	set_name("Human");
	set_power("Humanity");
	set_role(PlayerRole::MainPlayerOrAlly);
	set_mandatory(true);
	valid_phases_insert(TurnPhase::Reveal);
	set_description("As a main player or ally, after encounter cards are revealed, *use* this power to add 4 to your side's total. If this power is zapped, however, your side automatically wins the encounter.");
}

std::function<void()> Human::get_resolution_callback(GameState *g, const PlayerColors player, GameEvent &this_event, const GameEvent responding_to)
{
	std::function<void()> ret = [g,player,this_event] () { g->add_reinforcements(this_event,4,false); };
	return ret;
}

std::function<void()> Human::get_callback_if_countered(GameState *g, const PlayerColors player)
{
	std::function<void()> ret = [g,player] () { g->human_encounter_win_condition(); };
	return ret;
}
