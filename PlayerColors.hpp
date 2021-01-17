#pragma once

#include <string>

enum class PlayerColors
{
	Red,
	Blue,
	Purple,
	Yellow,
	Green,
	All, //Placeholder for events that don't focus on a particular player but need a valid playercolor
	Invalid
};

std::string to_string(const PlayerColors &p);
PlayerColors to_color(const std::string &s);

