#ifndef POKEMON_SHARED_H
#define POKEMON_SHARED_H

//#include <ctime>
//#include <sstream>

#include <chrono>
#include <string>

namespace PokemonGame
{
	using PokemonID = size_t;
	using PokemonName = std::string;

	using UserID = std::string;
	using UserPwd = std::string;
	using UserBadge = std::string;

	using SessionID = std::string;
	using RoomID = std::string;

	constexpr const char *BadSession = "You haven't Login";

	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

	struct TimePointHelper
	{
		static inline TimePoint TimeNow ()
		{
			return std::chrono::system_clock::now ();
		}

		static inline std::string ToStr (const TimePoint &timePoint)
		{
			//auto now_c = std::chrono::system_clock::to_time_t (timePoint);
			//std::ostringstream ss;
			//ss << std::put_time (std::gmtime (&now_c), "%Y_%m_%d_%H_%M_%S");
			//return ss.str ();

			auto msCount = std::chrono::duration_cast<
				std::chrono::milliseconds>(timePoint.time_since_epoch ()).count ();
			return std::to_string (msCount);
		}

		static inline TimePoint FromStr (const std::string &str)
		{
			//std::tm t;
			//std::istringstream ss (str);
			//ss >> std::get_time (&t, "%Y_%m_%d_%H_%M_%S");
			//auto tt = std::mktime (&t);
			//return std::chrono::system_clock::from_time_t (tt);

			auto msCount = std::stoll (str);
			return TimePoint (std::chrono::milliseconds (msCount));
		}
	};
}

#endif // !POKEMON_SHARED_H