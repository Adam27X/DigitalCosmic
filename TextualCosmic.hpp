#include <vector>

enum class PlayerColors
{
	Red,
	Blue,
	Purple,
	Yellow,
	Green
};

struct PlayerInfo
{
	PlayerColors color;
	unsigned score; //TODO: Provide a function to compute the score from the planet information below?
	//FIXME: Ships of different colors can reside on a planet, of course!
	std::vector< std::pair<PlayerColors,unsigned> > planets; //Each planet has some number of ships of each valid player color
};

class GameState
{
public:
	GameState(unsigned nplayers);
	void dump() const;
	PlayerInfo make_default_player(const PlayerColors color);
private:	
	unsigned num_players;
	std::vector<PlayerInfo> players;
};

