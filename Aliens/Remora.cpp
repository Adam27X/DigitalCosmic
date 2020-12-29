#include <cstdlib>
#include <iostream>

#include "Remora.hpp"
#include "GameState.hpp"

Remora::Remora()
{
	set_name("Remora");
	set_power("Cling");
	set_role(PlayerRole::AnyPlayer);
	set_mandatory(false);
	valid_phases_insert(TurnPhase::StartTurn);
	valid_phases_insert(TurnPhase::Regroup);
	valid_phases_insert(TurnPhase::Destiny);
	valid_phases_insert(TurnPhase::Launch);
	valid_phases_insert(TurnPhase::Alliance);
	valid_phases_insert(TurnPhase::Planning_before_selection);
	valid_phases_insert(TurnPhase::Planning_after_selection);
	valid_phases_insert(TurnPhase::Reveal);
	valid_phases_insert(TurnPhase::Resolution);
	set_description("Whenever another player retrives one more ships from the warp, you *may use* this power to retrieve one of your ships from the warp as well. You may not retrieve a ship from the warp during the asme encounter in which it went to the warp.\nWhenever another player draws one or more cards from the deck, you *may use* this power to draw one card from the deck as well.");
}

bool Remora::can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const
{
	if(!check_role_and_phase(e,t))
	{
		return false;
	}

	if(g.event_type == GameEventType::DrawCard || g.event_type == GameEventType::RetrieveWarpShip)
	{
		return (g.player != mycolor); //We can't respond to our own draws/ships taken from the warp
	}
	else
	{
		return false;
	}
}

std::function<void()> Remora::get_resolution_callback(GameState *g, const PlayerColors player, GameEvent &this_event, const GameEvent responding_to)
{
	std::function<void()> ret;
	if(responding_to.event_type == GameEventType::DrawCard)
	{
		ret = [g,player] () { g->draw_cosmic_card(g->get_player(player)); };
	}
	else if(responding_to.event_type == GameEventType::RetrieveWarpShip)
	{
		ret = [g,player] () { if(g->player_has_ship_in_warp_from_prior_encounter(player)) { g->move_ship_from_warp_to_colony(g->get_player(player)); } };
	}
	return ret;
}

