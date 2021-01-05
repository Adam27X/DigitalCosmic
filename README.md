# TextualCosmic
A digitial version of the game Cosmic Encounter.

## Philosophy

I started this project mostly as an interesting programming exercise. Cosmic Encounter is a unique game in which the rules of the game dynamically change based on the Aliens assigned to each player. The way in which the rules change complicate the resolution of game state effects which I found intriguing from a programming perspective.

Secondly, I should note that I am aware that Cosmic Encounter exists on Tabletop Simulator. TextualCosmic has slightly different goals from the game on Tabletop Simulator. Tabletop Simulator attempts to recreate the board game experience and requires users to physically move ships, draw cards, etc. While that design better reflects the experience of playing the game in person, TextualCosmic instead prefers to take advantage of its digital nature by controlling the game logic directly. Because TextualCosmic has direct knowledge of the game state, it can prompt players in the proper turn order for their actions. This design automatically resolves any disputes between players about turn phase ("are we in the Launch Phase or the Planning Phase right now? I want to play a card...") or turn order (e.g. two players both want to play a card at the same time, who gets to go first?).

Another distinction is that decisions in TextualCosmic are *binding*; the game will give you every opportunity to card zap your own card and if you perform that action despite not intending to, you can't "take it back." One could say this design philosophy promotes fairness at the expense of more explicit gameplay. For those of you who have played on Magic: The Gathering Arena, TextualCosmic aspires to provide a similar experience (i.e. MTG the trading card game is to MTG Arena as Cosmic Encounter is to TextualCosmic).

This game is called "TextualCosmic" at the moment because it initially was entirely text based and didn't provide any sort of GUI. That turned out to be a bad idea since players routinely need to reference parts of the game state (how many ships are on a given planet, what does the warp look like, which player has which alien, etc.). The game now has a GUI that certainly isn't pretty, but is good enough to contain this information. At the moment completing the game logic and supporting additional aliens and game variants are considered a higher priority to making the game look prettier. That said, if you do come across a problem with the GUI I would certainly like to be made aware of it!

## Building the project from source

Textual Cosmic consists of a server program that hosts the game and contains important game information and a client program that is used by all of the players in the game. Running the client is easy; just run client/python/TextualCosmicClient.py! I've used Python 3.8.6 for my testing but any recent version of Python should suffice.

TextualCosmic uses CMake to build the server program that hosts games, so building from source should follow a similar process regardless of your Operating System. CMake projects are at their cleanest with an out of tree build, so that's how the following instructions will proceed. Assume the current working directory is the root directory of the repository. The instructions below are for a Unix system but other OS's will follow a similar pattern.

### Unix

`$ mkdir build && cd build`

`$ cmake ..`

`$ make`

