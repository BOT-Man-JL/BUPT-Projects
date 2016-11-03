#ifndef POKEMON_SHARED_H
#define POKEMON_SHARED_H

#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>

#include <string>
#include <vector>
#include <unordered_map>

#include "Pokemon.h"

namespace PokemonGame_Impl
{
	// Data Types

	using PokemonID = size_t;
	using UserID = std::string;
	using SessionID = std::string;
	using RoomID = std::string;
	using PokemonsOfPlayer =
		std::vector<std::pair
		<PokemonID, std::unique_ptr<PokemonGame::Pokemon>> >;

	struct Player
	{
		static const size_t maxX = 300, maxY = 200;

		UserID uid;
		bool isReady;
		size_t x, y;
		PokemonsOfPlayer pokemons;
	};

	// Timestamp

	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

	std::string TimePointToStr (const TimePoint &now)
	{
		auto now_c = std::chrono::system_clock::to_time_t (now);
		std::ostringstream ss;
		ss << std::put_time (std::gmtime (&now_c), "%F %T");
		return ss.str ();
	}

	TimePoint TimePointFromStr (const std::string &str)
	{
		std::tm t;
		std::istringstream ss (str);
		ss >> std::get_time (&t, "%F %T");
		auto tt = std::mktime (&t);
		return std::chrono::system_clock::from_time_t (tt);
	}

	// Split Arguments

	std::vector<std::string> SplitStr (const std::string &input,
									   const std::string &delimiter)
	{
		std::vector<std::string> ret;
		size_t pos = 0;
		std::string inputStr = input;
		while ((pos = inputStr.find (delimiter)) != std::string::npos)
		{
			ret.push_back (inputStr.substr (0, pos));
			inputStr.erase (0, pos + delimiter.length ());
		}
		ret.push_back (inputStr);
		return std::move (ret);
	}
}

#endif // !POKEMON_SHARED_H