#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <sstream>

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

		case CosmicCardType::Attack42:
			ret = "Attack 42";
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
			ret = "Cosmic Zap";
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

		case CosmicCardType::Flare_TickTock:
			ret = "Flare: Tick-Tock";
		break;

		case CosmicCardType::Flare_Human:
			ret = "Flare: Human";
		break;

		case CosmicCardType::Flare_Remora:
			ret = "Flare: Remora";
		break;

		case CosmicCardType::Flare_Trader:
			ret = "Flare: Trader";
		break;

		case CosmicCardType::Flare_Sorcerer:
			ret = "Flare: Sorcerer";
		break;

		case CosmicCardType::Flare_Virus:
			ret = "Flare: Virus";
		break;

		case CosmicCardType::Flare_Spiff:
			ret = "Flare: Spiff";
		break;

		default:
			assert(0 && "Invalid Cosmic card type!");
		break;
	}

	return ret;
}

bool is_flare(const CosmicCardType c)
{
	return c >= CosmicCardType::Flare_TickTock && c <= CosmicCardType::None;
}

bool is_attack_card(const CosmicCardType c)
{
	return c >= CosmicCardType::Attack0 && c <= CosmicCardType::Attack42;
}

bool can_play_card_with_empty_stack(const TurnPhase state, const CosmicCardType c, const EncounterRole role, bool alien_enabled, const std::string &alien_name, const std::string &opponent_alien_name, bool &super_flare, const std::string &offense_alien_name, const std::string &defense_alien_name)
{
	switch(c)
	{
		case CosmicCardType::MobiusTubes:
			if(role == EncounterRole::Offense && state == TurnPhase::Regroup)
			{
				return true;
			}
			else
			{
				return false;
			}
		break;

		case CosmicCardType::Plague:
			//Valid for any player
			if(state == TurnPhase::Regroup)
			{
				return true;
			}
			else
			{
				return false;
			}
		break;

		case CosmicCardType::EmotionControl:
			if(state == TurnPhase::Reveal)
			{
				return true;
			}
			else
			{
				return false;
			}
		break;

		case CosmicCardType::ForceField:
			if(state == TurnPhase::Alliance_after_selection)
			{
				return true;
			}
			else
			{
				return false;
			}
		break;

		case CosmicCardType::Reinforcement2:
		case CosmicCardType::Reinforcement3:
		case CosmicCardType::Reinforcement5:
			if(state == TurnPhase::Reveal && role != EncounterRole::None) //Main player or ally only
			{
				return true;
			}
			else
			{
				return false;
			}
		break;

		case CosmicCardType::Quash:
			return false; //Handled as a response to a successful deal, as that's the only case in which it is valid to play a Quash
		break;

		case CosmicCardType::IonicGas:
			if(state == TurnPhase::Resolution)
			{
				return true;
			}
			else
			{
				return false;
			}
		break;

		case CosmicCardType::Flare_Human:
			//Wild flare for the Human; as a main player or ally during reveal, and can't be used by the Human because the super would be used instead (unless the human has lost their power or was zapped, but for the human I'm not sure if a zap response is possible). Finally, the flare states "if our opponent is not the Human", so check for that as well
			if(state == TurnPhase::Reveal && role != EncounterRole::None && (alien_name.compare("Human") != 0 || !alien_enabled) && opponent_alien_name.compare("Human") != 0)
			{
				//Wild
				return true;
			}
			else if(state == TurnPhase::Reveal && role != EncounterRole::None && alien_name.compare("Human") == 0 && alien_enabled)
			{
				//Super
				super_flare = true;
				return true;
			}
			else
			{
				return false;
			}
		break;

		case CosmicCardType::Flare_Trader:
			if(state == TurnPhase::Alliance_before_selection && (role == EncounterRole::Offense || role == EncounterRole::Defense) && (alien_name.compare("Trader") != 0 || !alien_enabled))
			{
				//Wild
				return true;
			}
			else if(state == TurnPhase::Planning_before_selection && (role == EncounterRole::Offense || role == EncounterRole::Defense) && alien_name.compare("Trader") == 0 && alien_enabled)
			{
				//Super
				super_flare = true;
				return true;
			}
			else
			{
				return false;
			}
		break;

		case CosmicCardType::Flare_Sorcerer:
			if(state == TurnPhase::Planning_before_selection && (alien_name.compare("Sorcerer") != 0 || !alien_enabled) && offense_alien_name.compare("Sorcerer") != 0 && defense_alien_name.compare("Sorcerer") != 0)
			{
				//Wild; can't be played if the Sorcerer is a main player
				return true;
			}
			else if(state == TurnPhase::Planning_after_selection && (role == EncounterRole::OffensiveAlly || role == EncounterRole::DefensiveAlly) && alien_name.compare("Sorcerer") == 0 && alien_enabled)
			{
				//Super
				super_flare = true;
				return true;
			}
			else
			{
				return false;
			}
		break;

		case CosmicCardType::Flare_Virus:
			if(state == TurnPhase::Reveal && (role == EncounterRole::Offense || role == EncounterRole::Defense) && (alien_name.compare("Virus") != 0 || !alien_enabled))
			{
				//Wild
				return true;
			}
			else if(state == TurnPhase::Reveal && (role == EncounterRole::Offense || role == EncounterRole::Defense) && alien_name.compare("Virus") == 0 && alien_enabled)
			{
				//Super
				super_flare = true;
				return true;
			}
			else
			{
				return false;
			}
		break;

		default:
			//Encounter cards, reinforcement cards, and zaps cannot be played with an empty stack
			//Quash can only be played in response to a deal
			//Ionic Gas should be played in response to compensation or defensive rewards
			return false;
		break;
	}
}

GameEventType to_game_event_type(const CosmicCardType c, bool super_flare)
{
	switch(c)
	{
		case CosmicCardType::MobiusTubes:
			return GameEventType::MobiusTubes;
		break;

		case CosmicCardType::Plague:
			return GameEventType::Plague;
		break;

		case CosmicCardType::EmotionControl:
			return GameEventType::EmotionControl;
		break;

		case CosmicCardType::ForceField:
			return GameEventType::ForceField;
		break;

		case CosmicCardType::Reinforcement2:
			return GameEventType::Reinforcement2;
		break;

		case CosmicCardType::Reinforcement3:
			return GameEventType::Reinforcement3;
		break;

		case CosmicCardType::Reinforcement5:
			return GameEventType::Reinforcement5;
		break;

		case CosmicCardType::Quash:
			return GameEventType::Quash;
		break;

		case CosmicCardType::IonicGas:
			return GameEventType::IonicGas;
		break;

		//NOTE: Only some flares exist here because only some flares can be played on an empty stack
		case CosmicCardType::Flare_Human:
			if(super_flare)
			{
				return GameEventType::Flare_Human_Super;
			}
			else
			{
				return GameEventType::Flare_Human_Wild;
			}
		break;

		case CosmicCardType::Flare_Trader:
			if(super_flare)
			{
				return GameEventType::Flare_Trader_Super;
			}
			else
			{
				return GameEventType::Flare_Trader_Wild;
			}
		break;

		case CosmicCardType::Flare_Sorcerer:
			if(super_flare)
			{
				return GameEventType::Flare_Sorcerer_Super;
			}
			else
			{
				return GameEventType::Flare_Sorcerer_Wild;
			}
		break;

		case CosmicCardType::Flare_Virus:
			if(super_flare)
			{
				return GameEventType::Flare_Virus_Super;
			}
			else
			{
				return GameEventType::Flare_Virus_Wild;
			}
		break;

		default:
			std::cerr << "Error: Unexpected CosmicCardType passed to to_game_event()\n";
			std::cerr << "Type: " << to_string(c) << "\n";
			assert(0);
		break;
	}
}

CosmicCardType to_cosmic_card_type(const GameEventType g)
{
	switch(g)
	{
		case GameEventType::CosmicZap:
			return CosmicCardType::CosmicZap;
		break;

		case GameEventType::CardZap:
			return CosmicCardType::CardZap;
		break;

		case GameEventType::MobiusTubes:
			return CosmicCardType::MobiusTubes;
		break;

		case GameEventType::Plague:
			return CosmicCardType::Plague;
		break;

		case GameEventType::EmotionControl:
			return CosmicCardType::EmotionControl;
		break;

		case GameEventType::ForceField:
			return CosmicCardType::ForceField;
		break;

		case GameEventType::Reinforcement2:
			return CosmicCardType::Reinforcement2;
		break;

		case GameEventType::Reinforcement3:
			return CosmicCardType::Reinforcement3;
		break;

		case GameEventType::Reinforcement5:
			return CosmicCardType::Reinforcement5;
		break;

		case GameEventType::Quash:
			return CosmicCardType::Quash;
		break;

		case GameEventType::IonicGas:
			return CosmicCardType::IonicGas;
		break;

		case GameEventType::Flare_Human_Wild:
		case GameEventType::Flare_Human_Super:
			return CosmicCardType::Flare_Human;
		break;

		case GameEventType::Flare_Trader_Wild:
		case GameEventType::Flare_Trader_Super:
			return CosmicCardType::Flare_Trader;
		break;

		case GameEventType::Flare_Sorcerer_Wild:
		case GameEventType::Flare_Sorcerer_Super:
			return CosmicCardType::Flare_Sorcerer;
		break;

		case GameEventType::Flare_Virus_Wild:
		case GameEventType::Flare_Virus_Super:
			return CosmicCardType::Flare_Virus;
		break;

		default:
			std::cerr << "Error: Unexpected GameEventType passed to to_cosmic_card_type()\n";
			std::cerr << "Type: " << to_string(g) << "\n";
			assert(0);
		break;
	}
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

	//Flares: TODO: Add in specific flares for each alien option provided to players (once we actually let players choose aliens)
	deck.insert(deck.end(),1,CosmicCardType::Flare_TickTock);
	deck.insert(deck.end(),1,CosmicCardType::Flare_Human);
	deck.insert(deck.end(),1,CosmicCardType::Flare_Remora);
	deck.insert(deck.end(),1,CosmicCardType::Flare_Trader);
	deck.insert(deck.end(),1,CosmicCardType::Flare_Sorcerer);
	deck.insert(deck.end(),1,CosmicCardType::Flare_Virus);
	deck.insert(deck.end(),1,CosmicCardType::Flare_Spiff);

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

