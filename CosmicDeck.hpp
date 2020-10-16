#pragma once

#include <vector>

enum class CosmicCardType
{
	Attack0 = 0, //1
	Attack1 = 1, //1
	Attack4 = 4, //4
	Attack5 = 5, //1
	Attack6 = 6, //7
	Attack7 = 7, //1
	Attack8 = 8, //7
	Attack9 = 9, //1
	Attack10 = 10, //4
	Attack11 = 11, //1
	Attack12 = 12, //2
	Attack13 = 13, //1
	Attack14 = 14, //2
	Attack15 = 15, //1
	Attack20 = 20, //2
	Attack23 = 23, //1
	Attack30 = 30, //1
	Attack40 = 40, //1
	Negotiate, //15
	Morph, //1
	Reinforcement2, //2
	Reinforcement3, //3
	Reinforcement5, //1
	CardZap, //2
	CosmicZap, //2
	MobiusTubes, //2
	EmotionControl, //1
	ForceField, //1
	IonicGas, //1
	Plague, //1
	Quash //1
};

std::string to_string(const CosmicCardType &c);

class CosmicDeck
{
public:
	//TODO: Support Flares
	CosmicDeck();
	void shuffle();
	void dump() const;
private:
	std::vector<CosmicCardType> deck;
};

