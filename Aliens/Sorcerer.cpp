#include <cstdlib>
#include <iostream>

#include "Sorcerer.hpp"

Sorcerer::Sorcerer()
{
	set_name("Sorcerer");
	set_power("Magic");
	set_role( PlayerRole::MainPlayer);
	set_mandatory(false);
	valid_phases_push_back(TurnPhase::Planning);
	set_description("As a main player, after encounter cards are selected, but before they are revealed, you *may use* this power to switch encounter cards with your opponent so that he or she reveals your card and you reveal your opponent's card.");
}

