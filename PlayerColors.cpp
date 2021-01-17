#include <cassert>
#include <iostream>

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

		case PlayerColors::All:
			ret = "All";
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

PlayerColors to_color(const std::string &s)
{
	PlayerColors ret;
	if(s.compare("Red") == 0)
	{
		ret = PlayerColors::Red;
	}
	else if(s.compare("Blue") == 0)
	{
		ret = PlayerColors::Blue;
	}
	else if(s.compare("Purple") == 0)
	{
		ret = PlayerColors::Purple;
	}
	else if(s.compare("Yellow") == 0)
	{
		ret = PlayerColors::Yellow;
	}
	else if(s.compare("Green") == 0)
	{
		ret = PlayerColors::Green;
	}
	else
	{
		ret = PlayerColors::Invalid;
	}

	return ret;
}
