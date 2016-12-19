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

	// Room Model
	struct RoomModel
	{
		RoomID rid;
		bool isPending;
	};

	// Room Player Model
	struct RoomPlayer
	{
		UserID uid;
		bool isReady;
		size_t width, height;
		PokemonModel pokemon;
	};

	// Game Model
	struct GameModel
	{
		struct GamePlayer
		{
			UserID uid;
			int x, y;
			int vx, vy;
			Pokemon::TimeGap timeGap;
			Pokemon::HealthPoint curHp;
		};

		struct GameDamage
		{
			int x, y;
			int vx, vy;
		};

		struct ResultPlayer
		{
			UserID uid;
			bool isWon;
		};

		std::vector<GamePlayer> players;
		std::unordered_map<std::string, GameDamage> damages;
		std::vector<ResultPlayer> results;
		bool isOver = false;
	};

	// Client
	class Client
	{
		using json = nlohmann::json;

	public:
		Client (const std::string &ipAddr,
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

		std::vector<RoomModel> Rooms ()
		{
			std::vector<RoomModel> ret;
			Request ("rooms", { { "sid", _sessionID } },
					 [&] (const json &response)
			{
				for (const auto &roomj : response)
					ret.emplace_back (_JsonToRoomModel (roomj));
			});
			return ret;
		}

		std::pair<size_t, size_t> RoomEnter (const RoomID &rid,
											 const PokemonID &pid)
		{
			std::pair<size_t, size_t> ret;
			Request ("roomenter", { { "sid", _sessionID },
									{ "rid", rid },
									{ "pid", pid } },
					 [&] (const json &response)
			{
				ret.first = response.at ("width").get<size_t> ();
				ret.second = response.at ("height").get<size_t> ();
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

		GameModel GameSync (
			const int movex, const int movey,
			const int atkx, const int atky, const bool isDef)
		{
			GameModel ret;
			Request ("gamesync", { { "sid", _sessionID },
								   { "movex", movex },
								   { "movey", movey },
								   { "atkx", atkx },
								   { "atky", atky },
								   { "def", isDef} },
					 [&] (const json &response)
			{
				ret.isOver = response.at ("over").get<bool> ();
				if (!ret.isOver)
				{
					for (const auto &playerj : response.at ("gameplayers"))
						ret.players.emplace_back (_JsonToGamePlayer (playerj));

					for (const auto &damagej : response.at ("gamedamages"))
						ret.damages.emplace (
							damagej.at ("did").get<std::string> (),
							_JsonToDamage (damagej)
						);
				}
				else
				{
					for (const auto &playerj : response.at ("resultplayers"))
						ret.results.emplace_back (_JsonToResultPlayer (playerj));
				}
			});
			return ret;
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

		RoomModel _JsonToRoomModel (const json &roomj)
		{
			return RoomModel {
				roomj.at ("rid").get<RoomID> (),
				roomj.at ("pending").get<bool> ()
			};
		}

		RoomPlayer _JsonToRoomPlayer (const json &playerj)
		{
			return RoomPlayer {
				playerj.at ("uid").get<UserID> (),
				playerj.at ("ready").get<bool> (),
				playerj.at ("width").get<size_t> (),
				playerj.at ("height").get<size_t> (),
				_JsonToPokemon (playerj.at ("pokemon"))
			};
		}

		GameModel::GamePlayer _JsonToGamePlayer (const json &playerj)
		{
			return GameModel::GamePlayer {
				playerj.at ("uid").get<UserID> (),
				playerj.at ("x").get<int> (),
				playerj.at ("y").get<int> (),
				playerj.at ("vx").get<int> (),
				playerj.at ("vy").get<int> (),
				playerj.at ("timegap").get<Pokemon::TimeGap> (),
				playerj.at ("hp").get<Pokemon::HealthPoint> ()
			};
		}

		GameModel::GameDamage _JsonToDamage (const json &damagej)
		{
			return GameModel::GameDamage {
				damagej.at ("x").get<int> (),
				damagej.at ("y").get<int> (),
				damagej.at ("vx").get<int> (),
				damagej.at ("vy").get<int> ()
			};
		}

		GameModel::ResultPlayer _JsonToResultPlayer (const json &playerj)
		{
			return GameModel::ResultPlayer {
				playerj.at ("uid").get<UserID> (),
				playerj.at ("won").get<bool> ()
			};
		}

		BOT_Socket::Client _sockClient;
		SessionID _sessionID;
	};
}

#endif // !POKEMON_CLIENT_H