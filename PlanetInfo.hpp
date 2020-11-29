#pragma once

#include <vector>
#include <functional>

//TODO: Rename this to PlanetInfo once it's fully working

//Wrapper around std::vector that has a side effect when pushing or erasing elements of the vector
//In this case the side effect is to send data to the client to update the GUI's display of the warp, hyperspace gate, etc.
template <class T>
class PlanetInfoFull
{
public:
	PlanetInfoFull(std::function<void()> callback) : server_callback(callback) { }

	void push_back(T item)
	{
		data.push_back(item);
		server_callback();
	}

	template <typename Iterator>
	Iterator erase(Iterator pos)
	{
		auto ret = data.erase(pos);
		server_callback();
		return ret;
	}

	unsigned size() const { return data.size(); }
	bool empty() const { return data.empty(); }

	typename std::vector<T>::const_iterator cbegin() const { return data.cbegin(); }
	typename std::vector<T>::const_iterator cend() const { return data.cend(); }
	typename std::vector<T>::iterator begin() { return data.begin(); }
	typename std::vector<T>::iterator end() { return data.end(); }

private:
	std::vector<T> data;
	std::function<void()> server_callback;
};
