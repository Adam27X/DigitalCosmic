#pragma once

#include <string>

enum class PlayerColors
{
	Red,
	Blue,
	Purple,
	Yellow,
	Green,
	Invalid
};

std::string to_string(const PlayerColors &p);
PlayerColors to_color(const std::string &s);

