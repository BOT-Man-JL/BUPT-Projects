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

namespace PokemonGame
{
	// Data Types

	using PokemonID = size_t;
	using UserID = std::string;
	using SessionID = std::string;
	using RoomID = std::string;

	using PokemonsOfPlayer =
		std::vector<std::pair
		<PokemonGame::PokemonID, std::unique_ptr<PokemonGame::Pokemon>> >;

	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

	enum class ActionType
	{
		None,
		Move,
		Attack,
		Defend,
		Recover,
		Switch
	};

	enum MoveDir
	{
		W, A, S, D
	};

	enum SwitchNo
	{
		First, Second, Third
	};

	struct Player
	{
		static const size_t maxX = 300, maxY = 200;

		struct Action
		{
			ActionType action;
			int param;
			TimePoint timestamp;
		};

		UserID uid;
		bool isReady;
		size_t x, y;
		PokemonsOfPlayer pokemons;
		std::vector<Action> actions;
	};
}

#define _HANDLE_ACTION_CASE(actionType, param, timestamp) \
case actionType:                                          \
return #actionType +                                      \
("\t" + std::to_string (param)) +                         \
("\t" + TimePointToStr (timestamp))                       \

#define _HANDLE_ACTION_STR(actionType, actionStr, param, timestamp)  \
if (actionStr == #actionType)                                        \
return PokemonGame::Player::Action                                   \
{ actionType, param, std::move (timestamp) }                         \

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
		auto now_c = std::chrono::system_clock::to_time_t (timePoint);
		std::ostringstream ss;
		ss << std::put_time (std::gmtime (&now_c), "%F %T");
		return ss.str ();
	}

	PokemonGame::TimePoint TimePointFromStr (const std::string &str)
	{
		std::tm t;
		std::istringstream ss (str);
		ss >> std::get_time (&t, "%F %T");
		auto tt = std::mktime (&t);
		return std::chrono::system_clock::from_time_t (tt);
	}

	// Action

	std::string ActionToStr (const PokemonGame::Player::Action &action)
	{
		using ActionType = PokemonGame::ActionType;

		switch (action.action)
		{
			_HANDLE_ACTION_CASE (ActionType::None, action.param, action.timestamp);
			_HANDLE_ACTION_CASE (ActionType::Move, action.param, action.timestamp);
			_HANDLE_ACTION_CASE (ActionType::Attack, action.param, action.timestamp);
			_HANDLE_ACTION_CASE (ActionType::Defend, action.param, action.timestamp);
			_HANDLE_ACTION_CASE (ActionType::Recover, action.param, action.timestamp);
			_HANDLE_ACTION_CASE (ActionType::Switch, action.param, action.timestamp);
		default:
			throw std::runtime_error ("WTF? (Will not hit)");
			break;
		}
	}

	PokemonGame::Player::Action ActionFromStr (const std::string &str)
	{
		using ActionType = PokemonGame::ActionType;

		auto args = SplitStr (str, "\t");
		if (args.size () < 3)
			throw std::runtime_error ("Bad Action String: " + str);
		auto param = std::stoi (args[1]);
		auto timestamp = TimePointFromStr (args[2]);

		_HANDLE_ACTION_STR (ActionType::None, args[0], param, timestamp);
		_HANDLE_ACTION_STR (ActionType::Move, args[0], param, timestamp);
		_HANDLE_ACTION_STR (ActionType::Attack, args[0], param, timestamp);
		_HANDLE_ACTION_STR (ActionType::Defend, args[0], param, timestamp);
		_HANDLE_ACTION_STR (ActionType::Recover, args[0], param, timestamp);
		_HANDLE_ACTION_STR (ActionType::Switch, args[0], param, timestamp);

		throw std::runtime_error ("Invalid Action");
	}
}

#endif // !POKEMON_SHARED_H