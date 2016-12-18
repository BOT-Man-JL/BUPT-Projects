#ifndef POKEMON_CLIENT_H
#define POKEMON_CLIENT_H

#include <string>
#include <vector>
#include <unordered_map>

#include "json/json.hpp"

#include "Socket.h"
#include "Shared.h"
#include "Pokemon.h"

namespace PokemonGame
{
	// Pokemon Model
	struct PokemonModel
	{
		PokemonID pid;
		UserID uid;

		PokemonName name;
		Pokemon::Level level;
		Pokemon::ExpPoint expPoint;
		Pokemon::HealthPoint atk;
		Pokemon::HealthPoint def;
		Pokemon::HealthPoint hp;
		Pokemon::TimeGap timeGap;
	};

	// User Model
	struct UserModel
	{
		UserID uid;
		bool online;
		double wonRate;
		std::vector<UserBadge> badges;
		std::vector<PokemonModel> pokemons;
	};

	// Room Player Model
	struct RoomPlayer
	{
		UserID uid;
		bool isReady;
		PokemonModel pokemon;
	};

	// Game Player Model
	struct GamePlayer
	{
		UserID uid;

		// Result Player
		bool isWon;

		// Game Player
		size_t x, y;
		size_t vx, vy;
		Pokemon::TimeGap timeGap;
		Pokemon::HealthPoint curHp;
	};

	// Client
	class PokemonClient
	{
		using json = nlohmann::json;

	public:
		PokemonClient (const std::string &ipAddr,
					   unsigned short port)
			: _sockClient (ipAddr, port)
		{}

#pragma region Accounting

		std::string Register (const UserID &uid,
							  const UserPwd &pwd)
		{
			std::string ret;
			Request ("register", { { "uid", uid },
								   { "pwd", pwd } },
					 [&] (const json &response)
			{
				ret = response.get<std::string> ();
			});
			return ret;
		}

		UserModel Login (const UserID &uid,
						 const UserPwd &pwd)
		{
			UserModel ret;
			Request ("login", { { "uid", uid },
								{ "pwd", pwd } },
					 [&] (const json &response)
			{
				_sessionID = response.at ("sid").get<SessionID> ();
				ret = _JsonToUser (response.at ("user"));
			});
			return ret;
		}

		std::string Logout ()
		{
			std::string ret;
			Request ("logout", { { "sid", _sessionID } },
					 [&] (const json &response)
			{
				ret = response.get<std::string> ();
			});
			return ret;
		}

#pragma endregion

#pragma region Query

		std::vector<PokemonModel> Pokemons ()
		{
			std::vector<PokemonModel> ret;
			Request ("pokemons", { { "sid", _sessionID } },
					 [&] (const json &response)
			{
				for (const auto &pokemonj : response)
					ret.emplace_back (_JsonToPokemon (pokemonj));
			});
			return ret;
		}

		std::vector<UserModel> Users ()
		{
			std::vector<UserModel> ret;
			Request ("users", { { "sid", _sessionID } },
					 [&] (const json &response)
			{
				for (const auto &userj : response)
					ret.emplace_back (_JsonToUser (userj));
			});
			return ret;
		}

#pragma endregion

#pragma region Room

		std::vector<RoomID> Rooms ()
		{
			std::vector<RoomID> ret;
			Request ("rooms", { { "sid", _sessionID } },
					 [&] (const json &response)
			{
				for (const auto &ridj : response)
					ret.emplace_back (ridj.get<RoomID> ());
			});
			return ret;
		}

		std::string RoomEnter (const RoomID &rid,
							   const PokemonID &pid)
		{
			std::string ret;
			Request ("roomenter", { { "sid", _sessionID },
									{ "rid", rid },
									{ "pid", pid } },
					 [&] (const json &response)
			{
				ret = response.get<std::string> ();
			});
			return ret;
		}

		std::string RoomLeave ()
		{
			std::string ret;
			Request ("roomleave", { { "sid", _sessionID } },
					 [&] (const json &response)
			{
				ret = response.get<std::string> ();
			});
			return ret;
		}

		std::vector<RoomPlayer> RoomReady (bool isReady)
		{
			std::vector<RoomPlayer> ret;
			Request ("roomready", { { "sid", _sessionID },
									{ "ready", isReady } },
					 [&] (const json &response)
			{
				for (const auto &playerj : response)
					ret.emplace_back (_JsonToRoomPlayer (playerj));
			});
			return ret;
		}

#pragma endregion

#pragma region Gaming

		std::pair<bool, std::vector<GamePlayer>> GameSync (
			const int movex, const int movey,
			const int atkx, const int atky, const bool isDef)
		{
			bool isOver;
			std::vector<GamePlayer> ret;
			Request ("gamesync", { { "sid", _sessionID },
								   { "movex", movex },
								   { "movey", movex },
								   { "atkx", atkx },
								   { "atky", atky },
								   { "def", isDef} },
					 [&] (const json &response)
			{
				isOver = response.at ("over").get<bool> ();
				if (!isOver)
				{
					for (const auto &playerj : response)
						ret.emplace_back (_JsonToGamePlayer (playerj));
				}
				else
				{
					for (const auto &playerj : response)
						ret.emplace_back (_JsonToResultPlayer (playerj));
				}
			});
			return std::make_pair (isOver, ret);
		}

#pragma endregion

	private:
		void Request (const std::string &request,
					  const json &param,
					  std::function<void (const json &response)> callback
					  = std::function<void (const json &response)> {})
		{
			try
			{
				auto response = _sockClient.Request (json {
					{ "request", request }, { "param", param }
				}.dump ());

				// Debug
				//std::cout << response << std::endl;

				const json res = json::parse (response.c_str ());
				if (!res.at ("success").get<bool> ())
					throw std::runtime_error (
						res.at ("response").get<std::string> ());

				if (callback) callback (res.at ("response"));
			}
			catch (const std::logic_error &)
			{
				throw std::runtime_error ("Bad Response");
			}
			catch (const std::runtime_error &)
			{
				throw;
			}
		}

		PokemonModel _JsonToPokemon (const json &pokemonj)
		{
			return PokemonModel {
				pokemonj.at ("pid").get<PokemonID> (),
				pokemonj.at ("uid").get<UserID> (),
				pokemonj.at ("name").get<PokemonName> (),
				pokemonj.at ("level").get<Pokemon::Level> (),
				pokemonj.at ("exppoint").get<Pokemon::ExpPoint> (),
				pokemonj.at ("atk").get<Pokemon::HealthPoint> (),
				pokemonj.at ("def").get<Pokemon::HealthPoint> (),
				pokemonj.at ("hp").get<Pokemon::HealthPoint> (),
				pokemonj.at ("timegap").get<Pokemon::TimeGap> ()
			};
		}

		UserModel _JsonToUser (const json &userj)
		{
			std::vector<UserBadge> badges;
			for (const auto &badgej : userj.at ("badges"))
				badges.emplace_back (badgej.get<UserBadge> ());

			std::vector<PokemonModel> pokemons;
			for (const auto &pokemonj : userj.at ("pokemons"))
				pokemons.emplace_back (_JsonToPokemon (pokemonj));

			return UserModel {
				userj.at ("uid").get<UserID> (),
				userj.at ("online").get<bool> (),
				userj.at ("wonrate").get<double> (),
				std::move (badges),
				std::move (pokemons)
			};;
		}

		RoomPlayer _JsonToRoomPlayer (const json &playerj)
		{
			return RoomPlayer {
				playerj.at ("uid").get<UserID> (),
				playerj.at ("ready").get<bool> (),
				_JsonToPokemon (playerj.at ("pokemon"))
			};
		}

		GamePlayer _JsonToGamePlayer (const json &playerj)
		{
			return GamePlayer {
				playerj.at ("uid").get<UserID> (),
				false,
				playerj.at ("x").get<size_t> (),
				playerj.at ("y").get<size_t> (),
				playerj.at ("vx").get<size_t> (),
				playerj.at ("vy").get<size_t> (),
				playerj.at ("timegap").get<Pokemon::TimeGap> (),
				playerj.at ("hp").get<Pokemon::HealthPoint> ()
			};
		}

		GamePlayer _JsonToResultPlayer (const json &playerj)
		{
			return GamePlayer {
				playerj.at ("uid").get<UserID> (),
				playerj.at ("won").get<bool> (),
				0, 0, 0, 0,
				Pokemon::TimeGap {}, Pokemon::HealthPoint {}
			};
		}

		BOT_Socket::Client _sockClient;
		SessionID _sessionID;
	};
}

#endif // !POKEMON_CLIENT_H