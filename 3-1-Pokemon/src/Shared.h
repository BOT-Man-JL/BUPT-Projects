#ifndef POKEMON_SHARED_H
#define POKEMON_SHARED_H

#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>

namespace PokemonGame
{
	using PokemonID = size_t;
	using PokemonName = std::string;

	using UserID = std::string;
	using UserPwd = std::string;
	using UserBadge = std::string;

	using SessionID = std::string;
	using RoomID = std::string;

	// Player

	struct Player
	{
		static constexpr size_t maxX = 300, maxY = 200;

		bool isReady;
		size_t x, y;
		Pokemon::TimeGap timeGap;
		PokemonID pid;
		std::unique_ptr<Pokemon> pokemon;
	};

	using Players = std::unordered_map<UserID, Player>;

	// Timestamp

	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

	std::string TimePointToStr (const TimePoint &timePoint)
	{
		//auto now_c = std::chrono::system_clock::to_time_t (timePoint);
		//std::ostringstream ss;
		//ss << std::put_time (std::gmtime (&now_c), "%Y_%m_%d_%H_%M_%S");
		//return ss.str ();

		auto msCount = std::chrono::duration_cast<
			std::chrono::milliseconds>(timePoint.time_since_epoch ()).count ();
		return std::to_string (msCount);
	}

	TimePoint TimePointFromStr (const std::string &str)
	{
		//std::tm t;
		//std::istringstream ss (str);
		//ss >> std::get_time (&t, "%Y_%m_%d_%H_%M_%S");
		//auto tt = std::mktime (&t);
		//return std::chrono::system_clock::from_time_t (tt);

		auto msCount = std::stoll (str);
		return TimePoint (std::chrono::milliseconds (msCount));
	}
}

#endif // !POKEMON_SHARED_H