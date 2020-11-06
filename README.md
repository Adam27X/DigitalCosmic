# TextualCosmic
A text-based version of the game Cosmic Encounter.

## Philosophy

I started this project mostly as an interesting programming exercise. Cosmic Encounter is a unique game in which the rules of the game dynamically change based on the Aliens assigned to each player. The way in which the rules change complicate the resolution of game state effects which I found intriguing from a programming perspective.

Secondly, I should note that I am aware that Cosmic Encounter exists on Tabletop Simulator. TextualCosmic has slightly different goals from the game on Tabletop Simulator. Tabletop Simulator attempts to recreate the board game experience and requires users to physically move ships, draw cards, etc. While that design better reflects the experience of playing the game in person, TextualCosmic instead prefers to take advantage of its digital nature by controlling the game logic directly. Because TextualCosmic has direct knowledge of the game state, it can prompt players in the proper turn order for their actions. This design automatically resolves any disputes between players about turn phase ("are we in the Launch Phase or the Planning Phase right now? I want to play a card...") or turn order (e.g. two players both want to play a card at the same time, who gets to go first?). Another distinction is that decisions in TextualCosmic are *binding*; the game will give you every opportunity to card zap your own card and if you perform that action despite not intending to, you can't "take it back." One could say this design philosophy promotes fairness at the expense of more explicit gameplay. For those of you who have played on Magic: The Gathering Arena, TextualCosmic aspires to provide a similar experience (i.e. MTG the trading card game is to MTG Arena as Cosmic Encounter is to TextualCosmic).

I call this version of the game "TextualCosmic" because, at the moment, the game is entirely text based and doesn't provide any sort of special GUI. That could change in the future, but I consider supporting additional game features to be a higher priority.

## Building the project from source

Support for Windows is currently limited. Only Cygwin has been tested, but MinGW or other alternatives may also work. Whether you use one of those methods or build on a Unix OS, the process should essentially be the same. TextualCosmic uses CMake and therefore prefers an out of tree build:

(Assume the current directory is the root directory of the repository)
`$ mkdir build && cd build`
`$ cmake ..`
`$ make`

Then to host a game server you can use `./textualcosmic` or to join an existing server you can use `./client/client`. Eventually I'll add support for installers for platforms of interest.
