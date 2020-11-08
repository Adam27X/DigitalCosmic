# TextualCosmic
A text-based version of the game Cosmic Encounter.

## Philosophy

I started this project mostly as an interesting programming exercise. Cosmic Encounter is a unique game in which the rules of the game dynamically change based on the Aliens assigned to each player. The way in which the rules change complicate the resolution of game state effects which I found intriguing from a programming perspective.

Secondly, I should note that I am aware that Cosmic Encounter exists on Tabletop Simulator. TextualCosmic has slightly different goals from the game on Tabletop Simulator. Tabletop Simulator attempts to recreate the board game experience and requires users to physically move ships, draw cards, etc. While that design better reflects the experience of playing the game in person, TextualCosmic instead prefers to take advantage of its digital nature by controlling the game logic directly. Because TextualCosmic has direct knowledge of the game state, it can prompt players in the proper turn order for their actions. This design automatically resolves any disputes between players about turn phase ("are we in the Launch Phase or the Planning Phase right now? I want to play a card...") or turn order (e.g. two players both want to play a card at the same time, who gets to go first?).

Another distinction is that decisions in TextualCosmic are *binding*; the game will give you every opportunity to card zap your own card and if you perform that action despite not intending to, you can't "take it back." One could say this design philosophy promotes fairness at the expense of more explicit gameplay. For those of you who have played on Magic: The Gathering Arena, TextualCosmic aspires to provide a similar experience (i.e. MTG the trading card game is to MTG Arena as Cosmic Encounter is to TextualCosmic).

I call this version of the game "TextualCosmic" because, at the moment, the game is entirely text based and doesn't provide any sort of special GUI. That could change in the future, but I consider supporting additional game features to be a higher priority.

## Building the project from source

TextualCosmic uses CMake, so building from source should follow a similar process regardless of OS. CMake projects are at their cleanest with an out of tree build, so that's how the following instructions will proceed. Assume the current working directory is the root directory of the repository.

So far CMake is the only significant dependency of this project. The allure of a proper GUI may change the project dependencies in the future.

### Windows (via Powershell)

Only the client build is currently supported natively on windows. Both the client and server can be built on Windows using Cygwin, however. In that case you can simply follow the Unix instructions below.

On Windows there are two build options: MinGW and Visual Studio.

### 1. MinGW

Be sure to add CMake and MinGW to your path in PowerShell:

`$ $env:Path += ";C:\Program Files\CMake\bin"`

`$ $env:Path += ";C:\MinGW\bin"`

`$ mkdir build && cd build`

`$ cmake .. -G "MinGW Makefiles"`

`$ mingw32-make client`

### 2. Visual Studio

This approach is very lightly tested so it's recommended users either use the MinGW approach above or obtain the client executable directly if possible. Still, the following steps should work:

Open the CMake GUI on Windows. Fill in the "Where is the source code:" text box with a path to the root of a checkout of the repository. Create a directory called 'build' under that directory and fill in the "Where to build the binaries:" with a path to this newly created directory.

Click configure, then assuming that succeeds click generate. Assuming the generation succeeds you can open the project in Visual Studio and then find 'client' in the Solution Explorer. Right-click client and choose Build to actually compile the code. Be sure to build it in "Release" mode so that the binary can be shared.

### Unix

`$ mkdir build && cd build`

`$ cmake ..`

`$ make`

## Running the project

To host a game server you can use `./textualcosmic` or to join an existing server you can use `./client/client`. For Visual Studio source builds you should be able to do this step from a PowerShell. Eventually I'll add support for installers for platforms of interest.
