#pragma once

#include <vector>
#include "AlienBase.hpp"

class TickTock : public AlienBase
{
public:
	TickTock();
	void discard_token();
private:
	unsigned num_tokens;
};

