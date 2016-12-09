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

#include "Pokemon.h"

namespace PokemonGame
{
	// Data Types

	using PokemonID = size_t;
	using UserID = std::string;
	using SessionID = std::string;
	using RoomID = std::string;

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

	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

	enum class ActionType
	{
		Move,
		Attack,
		Defend
	};

	struct Action
	{
		ActionType type;
		int x;
		int y;
		UserID uid;
		TimePoint timestamp;
	};

	struct ActionCmp
	{
		bool operator () (const Action &a, const Action &b)
		{
			return a.timestamp < b.timestamp;
		}
	};

	using ActionQueue =
		std::priority_queue<Action, std::deque<Action>, ActionCmp>;
}

#define _HANDLE_ACTION_CASE(actionType, x, y, uid, timestamp)  \
case actionType:                                               \
return #actionType +                                           \
("\t" + std::to_string (x)) +                                  \
("\t" + std::to_string (y)) +                                  \
("\t" + uid) +                                                 \
("\t" + TimePointToStr (timestamp))                            \

#define _HANDLE_ACTION_STR(actionType, actionStr, x, y, uid, timestamp)   \
if (actionStr == #actionType)                                             \
return PokemonGame::Action                                                \
{ actionType, x, y, std::move (uid), std::move (timestamp) }              \

namespace PokemonGame_Impl
{
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

	// Timestamp

	std::string TimePointToStr (const PokemonGame::TimePoint &timePoint)
	{
		//auto now_c = std::chrono::system_clock::to_time_t (timePoint);
		//std::ostringstream ss;
		//ss << std::put_time (std::gmtime (&now_c), "%Y_%m_%d_%H_%M_%S");
		//return ss.str ();

		auto msCount = std::chrono::duration_cast<
			std::chrono::milliseconds>(timePoint.time_since_epoch ()).count ();
		return std::to_string (msCount);
	}

	PokemonGame::TimePoint TimePointFromStr (const std::string &str)
	{
		//std::tm t;
		//std::istringstream ss (str);
		//ss >> std::get_time (&t, "%Y_%m_%d_%H_%M_%S");
		//auto tt = std::mktime (&t);
		//return std::chrono::system_clock::from_time_t (tt);

		auto msCount = std::stoll (str);
		return PokemonGame::TimePoint (std::chrono::milliseconds (msCount));
	}

	// Action

	std::string ActionToStr (const PokemonGame::Action &action)
	{
		using ActionType = PokemonGame::ActionType;

		switch (action.type)
		{
			_HANDLE_ACTION_CASE (ActionType::Move, action.x, action.y, action.uid, action.timestamp);
			_HANDLE_ACTION_CASE (ActionType::Attack, action.x, action.y, action.uid, action.timestamp);
			_HANDLE_ACTION_CASE (ActionType::Defend, action.x, action.y, action.uid, action.timestamp);
		default:
			throw std::runtime_error ("WTF? (Will not hit)");
			break;
		}
	}

	PokemonGame::Action ActionFromStr (const std::string &str)
	{
		using ActionType = PokemonGame::ActionType;

		auto args = SplitStr (str, "\t");
		if (args.size () < 5)
			throw std::runtime_error ("Bad Action String: " + str);
		auto x = std::stoi (args[1]);
		auto y = std::stoi (args[2]);
		auto &uid = args[3];
		auto timestamp = TimePointFromStr (args[4]);

		_HANDLE_ACTION_STR (ActionType::Move, args[0], x, y, uid, timestamp);
		_HANDLE_ACTION_STR (ActionType::Attack, args[0], x, y, uid, timestamp);
		_HANDLE_ACTION_STR (ActionType::Defend, args[0], x, y, uid, timestamp);

		throw std::runtime_error ("Invalid Action");
	}
}

#endif // !POKEMON_SHARED_H