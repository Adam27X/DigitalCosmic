#include <cstdlib>
#include <iostream>

#include "Remora.hpp"
#include "GameState.hpp"

Remora::Remora()
{
	set_name("Remora");
	set_power("Cling");
	set_role( PlayerRole::AnyPlayer);
	set_mandatory(false);
	valid_phases_push_back(TurnPhase::StartTurn);
	valid_phases_push_back(TurnPhase::Regroup);
	valid_phases_push_back(TurnPhase::Destiny);
	valid_phases_push_back(TurnPhase::Launch);
	valid_phases_push_back(TurnPhase::Alliance);
	valid_phases_push_back(TurnPhase::Planning_before_selection);
	valid_phases_push_back(TurnPhase::Planning_after_selection);
	valid_phases_push_back(TurnPhase::Reveal);
	valid_phases_push_back(TurnPhase::Resolution);
	set_description("Whenever another player retrives one more ships from the warp, you *may use* this power to retrieve one of your ships from the warp as well. You may not retrieve a ship from the warp during the asme encounter in which it went to the warp.\nWhenever another player draws one or more cards from the deck, you *may use* this power to draw one card from the deck as well.");
}

bool Remora::can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const
{
	if(g.event_type == GameEventType::DrawCard || g.event_type == GameEventType::RetrieveWarpShip)
	{
		return AlienBase::can_respond(e,t,g,mycolor) && (g.player != mycolor); //We can't respond to our own draws/ships taken from the warp
	}

	return false;
}

bool Remora::must_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const
{
	return false; //Remora is an optional Alien
}

std::function<void()> Remora::get_resolution_callback(GameState *g, const PlayerColors player)
{
	std::function<void()> ret = [g,player] () { g->draw_cosmic_card(g->get_player(player)); };
	return ret;
}
