#pragma once

#include <string>

enum class PlayerColors
{
	Red,
	Blue,
	Purple,
	Yellow,
	Green
};

std::string to_string(const PlayerColors &p);

