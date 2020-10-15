#include <vector>

enum class PlayerColors
{
	Red,
	Blue,
	Purple,
	Yellow,
	Green
};

typedef std::vector< std::pair<PlayerColors,unsigned> > PlanetInfo;

struct PlayerInfo
{
	PlayerColors color;
	unsigned score; //TODO: Provide a function to compute the score from the planet information below?
	//FIXME: Ships of different colors can reside on a planet, of course!
	std::vector<PlanetInfo> planets; //Each planet has some number of ships of each valid player color
};

enum class DestinyCardType
{
	Red, //3 for each player
	Blue,
	Purple,
	Yellow,
	Green,
	Special_fewest_ships_in_warp, //1
	Special_most_cards_in_hand, //1
	Special_most_foreign_colonies, //1
	Wild //2

};

class DestinyDeck
{
public:
	DestinyDeck(unsigned nplayers);
	void shuffle();
	void dump() const;
private:
	std::vector<DestinyCardType> deck;
};

class GameState
{
public:
	GameState(unsigned nplayers);
	void dump() const;
	PlayerInfo make_default_player(const PlayerColors color);
	void dump_destiny_deck() const;
private:	
	void shuffle_destiny_deck();
	unsigned num_players;
	std::vector<PlayerInfo> players;
	DestinyDeck destiny_deck;
};

