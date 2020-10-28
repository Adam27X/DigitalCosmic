#include <cstdlib>
#include <iostream>

#include "TickTock.hpp"

//TODO: If we implement a 4 planet variant then TickTock should only start with 8 tokens
TickTock::TickTock() : num_tokens(10)
{
	set_name("Tick-Tock");
	set_power("Patience");
	set_role( PlayerRole::AnyPlayer);
	set_mandatory(true);
	valid_phases_push_back(TurnPhase::Resolution);
	set_description("You start the game with ten tokens. Each time any player wins an encounter as the defense or a successful deal is made between any two players, *use* this power to discard one token. If you have no more tokens, you immediately win the game. You may still win the game via the normal method.");
}

void TickTock::discard_token()
{
	num_tokens--;
	std::cout << "Number of Tick-Tock tokens remaining: " << num_tokens << "\n";
	if(num_tokens == 0)
	{
		std::cout << "Tick-Tock has won the game!\n";
		std::exit(0); //TODO: Propagate this info up the stack to exit properly
	}
}

bool TickTock::can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const
{
	if(g.event_type == GameEventType::SuccessfulDeal || g.event_type == GameEventType::DefensiveEncounterWin)
	{
		return AlienBase::can_respond(e,t,g,mycolor); //Should always be true
	}
	return false;
}

bool TickTock::must_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const
{
	return false; //FIXME: HACK since this Alien is NYI...change to true once we're done
}

std::function<void()> TickTock::get_resolution_callback(GameState *g, const PlayerColors player)
{
	std::function<void()> ret = [this] () { this->discard_token(); };
	return ret;
}
