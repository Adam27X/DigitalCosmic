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

		default:
			assert(0 && "Invalid Cosmic card type!");
		break;
	}

	return ret;
}

bool can_play_card_with_empty_stack(const TurnPhase state, const CosmicCardType c, const EncounterRole role)
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
			if(state == TurnPhase::Alliance)
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

		default:
			//Encounter cards, reinforcement cards, and zaps cannot be played with an empty stack
			//Quash can only be played in response to a deal
			//Ionic Gas should be played in response to compensation or defensive rewards
			return false;
		break;
	}
}

GameEventType to_game_event_type(const CosmicCardType c)
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

		default:
			std::cerr << "Error: Unexpected GameEventType passed to to_cosmic_card_type()\n";
			std::cerr << "Type: " << to_string(g) << "\n";
			assert(0);
		break;
	}
}

std::string card_info(const CosmicCardType c)
{
	std::stringstream ret;
	if(static_cast<unsigned>(c) <= 40) 
	{
		ret << "Attack N";
	}
	else if(c == CosmicCardType::Reinforcement2 || c == CosmicCardType::Reinforcement3 || c == CosmicCardType::Reinforcement5)
	{
		ret << "Reinforcement +N";
	}
	else
	{
		ret << to_string(c);
	}
	ret << ": ";

	if(static_cast<unsigned>(c) <= 40) //Attack card
	{
		ret << "[Encounter card] Opposed by attack: Higher total (ships + N) wins. Opposed by Negotiate: Wins, but opponent collects compensation.";
	}
	else if(c == CosmicCardType::Negotiate)
	{
		ret << "[Encounter card] Opposed by attack: Loses, but collects compensation. Opposed by Negotiate: Players have one minute to make a deal or lose three ships to the warp.";
	}
	else if(c == CosmicCardType::Morph)
	{
		ret << "[Encounter card] Duplicates opponent's encounter card when revealed.";
	}
	else if(c == CosmicCardType::Reinforcement2 || c == CosmicCardType::Reinforcement3 || c == CosmicCardType::Reinforcement5)
	{
		ret << "[Reinforcement card] Adds N to either side's total. Play after encounter cards are revealed. [Play as a main player or ally only] [Play during the reveal phase only]";
	}
	else if(c == CosmicCardType::CardZap)
	{
		ret << "[Artifact card] Negates Cards. Play this card at any time to negate a flare or artifact card just as a player attempts to use it. The flare or artifact must then be discarded. [As any player] [During any turn phase]";
	}
	else if(c == CosmicCardType::CosmicZap)
	{
		ret << "[Artifact card] Stops Power. Play this card at any time to cancel one *use* of any alien's power, including your own. That power may not be used again during the current encounter. [As any player] [During any turn phase]";
	}
	else if(c == CosmicCardType::MobiusTubes)
	{
		ret << "[Artifact card] Frees Ships. Play at the start of one of your encounters to free all ships from the warp. Freed ships may return to any of their owners' colonies. [Play as the offense only] [ Play during the regroup phase only]";
	}
	else if(c == CosmicCardType::Plague)
	{
		ret << "[Artifact card] Harms Player. Play at the start of any encounter and choose a player. That player loses three ships of his or her choice to the warp (if possible) and must discard one card of each type that he or she has in hand (such as attack, negotiate, artifact, flare, etc.). [As any player] [Play during the regroup phase only]";
	}
	else if(c == CosmicCardType::ForceField)
	{
		ret << "[Artifact card] Stops Allies. Play after alliances are formed during an encounter. You may cancel the alliances of any or all players. Cancelled allies return their ships to any of their colonies. [As any player] [Play during the alliance phase only]";
	}
	else if(c == CosmicCardType::EmotionControl)
	{
		ret << "[Artifact card] Alters Attack. Play after encounter cards are revealed to treat all attack cards played this encounter as negotiate cards. The main players must then attempt to make a deal. [As any player] [Play during the reveal phase only]";
	}
	else if(c == CosmicCardType::Quash)
	{
		ret << "[Artifact card] Kills Deal. Play after a deal is made successfully. Cancel the deal, and the dealing players suffer the penalties for a failed deal. [As any player] [Play during the resolution phase only]";
	}
	else if(c == CosmicCardType::IonicGas)
	{
		ret << "[Artifact card] Stops Compensation and Rewards. Play after the winner of an encounter is determined. No compensation or defensive rewards may be collected this encounter. [As any player] [Play during the resolution phase only]";
	}
	else
	{
		std::cerr << "Unexpected CosmicCardType passed to card_info(): " << to_string(c) << "\n";
		assert(0);
	}

	ret << "\n";
	return ret.str();
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

