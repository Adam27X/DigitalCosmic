#pragma once

#include <vector>
#include <string>

#include "TickTock.hpp"
#include "Human.hpp"
#include "Machine.hpp"
#include "Remora.hpp"
#include "Trader.hpp"
#include "Sorcerer.hpp"
#include "Virus.hpp"
#include "Spiff.hpp"
#include "Shadow.hpp"
#include "Warpish.hpp"
#include "Oracle.hpp"

//NOTE: Using vector instead of set here so we can easily grab elements at random via std::random_shuffle
static const std::vector<std::string> &available_aliens = {"TickTock","Human","Remora","Trader","Sorcerer","Virus","Spiff","Machine","Shadow","Warpish","Oracle"};

