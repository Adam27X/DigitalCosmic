#include <cassert>

#include "PlayerInfo.hpp"

std::string to_string(const PlayerColors &p)
{
	std::string ret;
	switch(p)
	{
		case PlayerColors::Red:
			ret = "Red";
		break;

		case PlayerColors::Blue:
			ret = "Blue";
		break;

		case PlayerColors::Purple:
			ret = "Purple";
		break;

		case PlayerColors::Yellow:
			ret = "Yellow";
		break;

		case PlayerColors::Green:
			ret = "Green";
		break;

		case PlayerColors::Invalid:
			assert(0 && "Attempt to print string for invalid Player color!");
		break;

		default:
			assert(0 && "Invalid player color!");
		break;
	}

	return ret;
}

