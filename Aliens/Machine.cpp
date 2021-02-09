#include <cstdlib>
#include <iostream>

#include "Machine.hpp"
#include "GameState.hpp"

Machine::Machine()
{
	set_name("Machine");
	set_power("Continuity");
	set_role(PlayerRole::Offense);
	set_mandatory(false);
	valid_phases_insert(TurnPhase::Resolution);
	set_description("Your turn is not limited to two encounters. After completing an encounter (whether successful or not), you may use this power to have another encounter as long as you have at least one encounter card left in your hand.");
}

std::function<void()> Machine::get_resolution_callback(GameState *g, const PlayerColors player, GameEvent &this_event, const GameEvent responding_to)
{
	std::function<void()> ret = [g] () { g->set_machine_continues_turn(); };
	return ret;
}

