# DigitalCosmic
A digitial version of the board game Cosmic Encounter.

## Philosophy

I started this project mostly as an interesting programming exercise. Cosmic Encounter is a unique game in which the rules of the game dynamically change based on the Aliens assigned to each player. The way in which the rules change complicate the resolution of game state effects which I found intriguing from a programming perspective.

Cosmic Encounter technically already exists in a digital format on Tabletop Simulator. However, this project has slightly different goals from the Tabletop Simulator version of the game. Tabletop Simulator attempts to recreate the board game experience; it requires users to physically click and drag ships and cards to move them. While that design better reflects the experience of playing the game in person, DigitalCosmic instead prefers to exploit its digital nature by controlling the game logic directly. Because DigitalCosmic has direct knowledge of the game state, it can prompt players in the proper turn order for their actions. This design automatically resolves any disputes between players about turn phase ("are we in the Launch Phase or the Planning Phase right now? I want to play a card...") or turn order (e.g. two players both want to play a card at the same time, who gets to go first?). For the Tabletop Simulator version of the game, these decisions require human intervention and at times these decisions are error prone. This design philosophy was haveily inspired by Magic the Gathering: Arena.

This game was initially called "TextualCosmic" because it was entirely text based and didn't provide any sort of GUI. That turned out to be a bad idea since players routinely need to reference parts of the game state (how many ships are on a given planet, what does the warp look like, which player has which alien, etc.), which is inconvenient in text format. The game now has a GUI that certainly isn't pretty, but is good enough to contain this information. At the moment completing the game logic and supporting additional aliens and game variants are considered a higher priority to making the game look prettier. That said, if you do come across a problem with the GUI I would certainly like to be made aware of it!

## Setting up the game

DigitalCosmic uses a client-server model. To start a game someone hosts the server, which has the responsibility of computing game logic and processing requests and responses from clients (players). Each player executes the client script, which creates a GUI that tells the player things like which cards they have in their hand, their alien power, and how many ships are on each planet. Although the server program has been carefully designed and packaged for simplicity it may be a good idea to let the more "technologically inclined" players handle hosting the server in case you run into any issues.

## Running the client

Once the server is running, each player needs to run the client program and connect to it. The client program is a simple Python script and should work on Windows, Mac, or Linux systems. The client program depends on Python 3, Tcl, and Tk. I used Python 3.8.8 for most of my testing but any somewhat recent version of Python should work just fine and Tcl/Tk should be bundled with Python and/or exist on most Windows/Mac/Linux systems to begin with.

Navigate to your installation of DigitalCosmic and from that directory run navigate to client/python and run DigitalCosmicClient.py. A window should pop up with fields for the server IP address and port. The host of the server can provide you with this information or obtain it from resources such as whatismyip.com. The default server port is 3074 but the server host can choose to use a different port if they wish. Note that if all of the players are playing on the same local network then the server's local address (such as 192.168.x.y) will also work.

Once all of the players connect to the server the game GUI will populate and the server will start the game. Good luck!

## Hosting the server

Hosting the server is a bit more complicated, but only needs to be done once per game. For Windows users a prebuilt executable program is supplied in this repository under Releases. In that directory find the path that corresponds to the most recent release and navigate to the Win32 directory. Inside that directory there is an installer that will install the server program for you. When the server program is installed you can skip ahead to the section entitled __Running the server__.

For players that prefer to build directly from source or players on Mac or Linux, see the next section for how to build the server program.

## Building the server program from source

DigitalCosmic uses CMake to compile the server source code. The general flow will be the same regardless of OS, but each OS may have its own idiosyncracies on certain steps. CMake projects are best built using an 'out of tree' build, which is the process we will follow below. Assume the current working directory is the root directory of this repository.

### Windows

There are a few ways to build the code from source on Windows. Using MinGW-w64 is my preferred approach and is the approach that will be outlined in this section. Cygwin has also been tested; Cygwin users should follow the Mac and Linux instructions below. Visual Studio is not officially supported (this is what happens when your compiler isn't freely available...), but with a few tweaks to the CMakeLists.txt files used for this project could probably be made to work as well. If you use Visual Studio and you're able to get the server build working, please send me a patch!

Start by opening a powershell and navigating to this repository. Make a build directory and cd to it:

`> mkdir build_win32`

`> cd build_win32`

Run CMake to create the build files for MinGW:

`> cmake .. -G "MinGW Makefiles"`

Run Make to compile the source:

`> mingw32-make`

Once all of the source is compiled and linked you should see a line that looks something like '[100%] Built target digitalcosmic'. Success! At this point you can run the following command to see the available server options:

`> .\digitalcosmic.exe -h`

Proceed to __Runing the server__ for further instructions on how to start the game.

### Mac and Linux

On Unix systems the CMake build flow used here is very standard. Start by opening a terminal and navigating to this repository. Make a build directory and cd to it:

`$ mkdir build && cd build`

Run CMake to create the build files:

`$ cmake ..`

Run Make to compile the source:

`$ make`

Once all of the source is compiled and linked you should see a line that looks something like '[100%] Built target digitalcosmic'. Success! At this point you can run the following command to see the available server options:

`$ ./digitalcosmic -h`

Proceed to __Runing the server__ for further instructions on how to start the game.

## Running the server

The server takes in various options to customize the game. The only required option as of this writing is the number of players, passed via the `-n` or `--num_players` argument. So, to start a four player game you can do the following (Windows)

`> .\digitalcosmic.exe -n 4`

or similarly on Mac/Linux:

`$ ./digitalcosmic -n 4`

If you're on Windows and are using the prebuilt binary (and thus didn't have to build the source), you can also navigate to the digitalcosmic installation directory and then to the bin directory (for me this looks like C:\Program Files (x86)\digitalcosmic 0.02.1\bin). From that directory you can start the server directly from Windows Explorer by typing the following in the address bar:

`digitalcosmic.exe -n 4`

Once the server is started it will wait for each player to connect via the client program and then it will start the game. Good luck and please let me know if you run into issues!
