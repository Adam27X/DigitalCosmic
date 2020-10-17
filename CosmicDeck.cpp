#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>

#include "CosmicDeck.hpp"

std::string to_string(const CosmicCardType &c)
{
	std::string ret;
	switch(c)
	{
		case CosmicCardType::Attack0:
			ret = "Attack 0";
		break;

		case CosmicCardType::Attack1:
			ret = "Attack 1";
		break;

		case CosmicCardType::Attack4:
			ret = "Attack 4";
		break;

		case CosmicCardType::Attack5:
			ret = "Attack 5";
		break;

		case CosmicCardType::Attack6:
			ret = "Attack 6";
		break;

		case CosmicCardType::Attack7:
			ret = "Attack 7";
		break;

		case CosmicCardType::Attack8:
			ret = "Attack 8";
		break;

		case CosmicCardType::Attack9:
			ret = "Attack 9";
		break;

		case CosmicCardType::Attack10:
			ret = "Attack 10";
		break;

		case CosmicCardType::Attack11:
			ret = "Attack 11";
		break;

		case CosmicCardType::Attack12:
			ret = "Attack 12";
		break;

		case CosmicCardType::Attack13:
			ret = "Attack 13";
		break;

		case CosmicCardType::Attack14:
			ret = "Attack 14";
		break;

		case CosmicCardType::Attack15:
			ret = "Attack 15";
		break;

		case CosmicCardType::Attack20:
			ret = "Attack 20";
		break;

		case CosmicCardType::Attack23:
			ret = "Attack 23";
		break;

		case CosmicCardType::Attack30:
			ret = "Attack 30";
		break;

		case CosmicCardType::Attack40:
			ret = "Attack 40";
		break;

		case CosmicCardType::Negotiate:
			ret = "Negotiate";
		break;

		case CosmicCardType::Morph:
			ret = "Morph";
		break;

		case CosmicCardType::Reinforcement2:
			ret = "Reinforcement +2";
		break;

		case CosmicCardType::Reinforcement3:
			ret = "Reinforcement +3";
		break;

		case CosmicCardType::Reinforcement5:
			ret = "Reinforcement +5";
		break;

		case CosmicCardType::CardZap:
			ret = "Card Zap";
		break;

		case CosmicCardType::CosmicZap:
			ret = "CosmicZap";
		break;

		case CosmicCardType::MobiusTubes:
			ret = "Mobius Tubes";
		break;

		case CosmicCardType::EmotionControl:
			ret = "Emotion Control";
		break;

		case CosmicCardType::ForceField:
			ret = "Force Field";
		break;

		case CosmicCardType::IonicGas:
			ret = "Ionic Gas";
		break;

		case CosmicCardType::Plague:
			ret = "Plague";
		break;

		case CosmicCardType::Quash:
			ret = "Quash";
		break;

		default:
			assert(0 && "Invalid Destiny card type!");
		break;
	}

	return ret;
}

CosmicDeck::CosmicDeck()
{
	deck.insert(deck.end(),1,CosmicCardType::Attack0);
	deck.insert(deck.end(),1,CosmicCardType::Attack1);
	deck.insert(deck.end(),4,CosmicCardType::Attack4);
	deck.insert(deck.end(),1,CosmicCardType::Attack5);
	deck.insert(deck.end(),7,CosmicCardType::Attack6);
	deck.insert(deck.end(),1,CosmicCardType::Attack7);
	deck.insert(deck.end(),7,CosmicCardType::Attack8);
	deck.insert(deck.end(),1,CosmicCardType::Attack9);
	deck.insert(deck.end(),4,CosmicCardType::Attack10);
	deck.insert(deck.end(),1,CosmicCardType::Attack11);
	deck.insert(deck.end(),2,CosmicCardType::Attack12);
	deck.insert(deck.end(),1,CosmicCardType::Attack13);
	deck.insert(deck.end(),2,CosmicCardType::Attack14);
	deck.insert(deck.end(),1,CosmicCardType::Attack15);
	deck.insert(deck.end(),2,CosmicCardType::Attack20);
	deck.insert(deck.end(),1,CosmicCardType::Attack23);
	deck.insert(deck.end(),1,CosmicCardType::Attack30);
	deck.insert(deck.end(),1,CosmicCardType::Attack40);
	deck.insert(deck.end(),15,CosmicCardType::Negotiate);
	deck.insert(deck.end(),1,CosmicCardType::Morph);
	deck.insert(deck.end(),2,CosmicCardType::Reinforcement2);
	deck.insert(deck.end(),3,CosmicCardType::Reinforcement3);
	deck.insert(deck.end(),1,CosmicCardType::Reinforcement5);
	deck.insert(deck.end(),2,CosmicCardType::CardZap);
	deck.insert(deck.end(),2,CosmicCardType::CosmicZap);
	deck.insert(deck.end(),2,CosmicCardType::MobiusTubes);
	deck.insert(deck.end(),1,CosmicCardType::EmotionControl);
	deck.insert(deck.end(),1,CosmicCardType::ForceField);
	deck.insert(deck.end(),1,CosmicCardType::IonicGas);
	deck.insert(deck.end(),1,CosmicCardType::Plague);
	deck.insert(deck.end(),1,CosmicCardType::Quash);

	shuffle();
}

void CosmicDeck::shuffle()
{
	std::random_shuffle(deck.begin(),deck.end());
}

void CosmicDeck::dump() const
{
	std::cout << "Cosmic deck:\n";
	for(auto i=deck.begin(),e=deck.end();i!=e;++i)
	{
		std::cout << to_string(*i) << "\n";
	}
}
