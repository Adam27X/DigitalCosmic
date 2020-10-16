#include <cstdlib>
#include <iostream>

#include "Human.hpp"

Human::Human()
{
	set_name("Human");
	set_power("Humanity");
	set_role(PlayerRole::MainPlayerOrAlly);
	set_mandatory(true);
	valid_phases_push_back(TurnPhase::Reveal);
	set_description("As a main player or ally, after encounter cards are revealed, *use* this power to add 4 to your side's total. If this power is zapped, however, your side automatically wins the encounter.");
}

