#include <cstdlib>
#include <iostream>

#include "Trader.hpp"

Trader::Trader()
{
	set_name("Trader");
	set_power("Transference");
	set_role( PlayerRole::MainPlayer);
	set_mandatory(false);
	valid_phases_push_back(TurnPhase::Planning_before_selection);
	//Use std::swap for the trade?
	set_description("As a main player, before encounter cards are selected, you *may use* this power to trade hands with your opponent. You each then keep the new hand.");
}

