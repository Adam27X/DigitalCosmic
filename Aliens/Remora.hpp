#pragma once

#include <vector>
#include "AlienBase.hpp"

class Remora : public AlienBase
{
public:
	Remora();
	bool can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const override;
};

