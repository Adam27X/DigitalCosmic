#pragma once

#include <vector>
#include "AlienBase.hpp"

class TickTock : public AlienBase
{
public:
	TickTock();
	void discard_token();
	bool can_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const override;
	bool must_respond(EncounterRole e, TurnPhase t, GameEvent g, PlayerColors mycolor) const override;
private:
	unsigned num_tokens;
};

