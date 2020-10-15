enum class PlayerColors
{
	Red,
	Blue,
	Purple,
	Yellow,
	Green
};

class GameState
{
public:
	GameState(unsigned nplayers);
	void dump() const;
private:	
	unsigned num_players;
	std::map<PlayerColors,unsigned> scores;
	std::map<PlayerColors, std::vector<unsigned> > planets; //How many ships on each player's planet? Eventually we'll want a more embellished struct to represent planet information
};

