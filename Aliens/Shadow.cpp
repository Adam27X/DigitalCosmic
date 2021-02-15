#include <cstdlib>
#include <iostream>

#include "Shadow.hpp"
#include "GameState.hpp"

Shadow::Shadow()
{
	set_name("Shadow");
	set_power("Execution");
	set_role(PlayerRole::AnyPlayer);
	set_mandatory(true);
	valid_phases_insert(TurnPhase::Destiny);
	set_description("Whenever any other player's color or a special destiny card that targets another player is drawn from the destiny deck, use this power to choose one of that player's ships from any colony of your choice and send it to the warp. On a wild destiny card, you may execute one ship belonging to any other player regardless of who the offense chooses to attack.");
}

bool Shadow::can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const
{
	if(!check_role_and_phase(e,t))
	{
		return false;
	}

	if((g.event_type == GameEventType::DestinyCardDrawn && g.player != mycolor) || g.event_type == GameEventType::DestinyWildDrawn)
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::function<void()> Shadow::get_resolution_callback(GameState *g, const PlayerColors player, GameEvent &this_event, const GameEvent responding_to)
{
	std::function<void()> ret;
	if(responding_to.event_type == GameEventType::DestinyCardDrawn)
	{
		ret = [g,player,responding_to] () { g->execute_ship(player,responding_to.player); };
	}
	else if(responding_to.event_type == GameEventType::DestinyWildDrawn)
	{
		ret = [g,player] () { g->execute_ship(player,PlayerColors::Invalid); };
	}
	return ret;
}

