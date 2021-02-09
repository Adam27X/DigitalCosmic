#include <cstdlib>
#include <iostream>

#include "Spiff.hpp"
#include "GameState.hpp"

Spiff::Spiff()
{
	set_name("Spiff");
	set_power("Crash Land");
	set_role(PlayerRole::Offense);
	set_mandatory(false);
	valid_phases_insert(TurnPhase::Resolution);
	set_description("As the offense, when you lose an encounter, if both players revealed attack cards and you lost the encounter by 10 or more, you may use this power to land one of your ships that would otherwise be lost to the wrap on the winning defensive planet. The ship coexists with the ships already there. This power does not allow you to coexist in places or with aliens that state otherwise.");
}

bool Spiff::can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const
{
	if(!check_role_and_phase(e,t))
	{
		return false;
	}

	if(g.event_type == GameEventType::CrashLandTrigger && g.player == mycolor)
	{
		return true; //The player of the CrashLandTrigger event type is the offense and we need to be the offense for this response to be valid
	}
	else
	{
		return false;
	}
}

std::function<void()> Spiff::get_resolution_callback(GameState *g, const PlayerColors player, GameEvent &this_event, const GameEvent responding_to)
{
	std::function<void()> ret = [g,player] () { g->possible_crash_landing(player); };
	return ret;
}

