#ifndef POKEMON_SERVER_H
#define POKEMON_SERVER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <memory>
#include <chrono>
#include <algorithm>

#include "ORM-Lite/ORMLite.h"
#include "json/json.hpp"

#include "Socket.h"
#include "Shared.h"
#include "Pokemon.h"

//#include "PokemonPhysics.h"

// Fix for the pollution by <windows.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

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

		inline auto ToPokemon ()
		{
			return Pokemon::NewPokemon (
				name, level, expPoint,
				atk, def, hp, timeGap);
		}

		static auto FromPokemon (
			PokemonID pid, UserID uid,
			const Pokemon &pokemon)
		{
			return std::unique_ptr <PokemonModel> {
				new PokemonModel { pid, uid, pokemon.GetName (),
					pokemon.GetLevel (), pokemon.GetExp (),
					pokemon.GetAtk (), pokemon.GetDef (),
					pokemon.GetHP (), pokemon.GetTimeGap ()
				}};
		}

		ORMAP ("Pokemon", pid, uid, name, level, expPoint,
			   atk, def, hp, timeGap);
	};

	// Badge Model
	struct BadgeModel
	{
		size_t bid;
		UserID uid;
		UserBadge badge;

		ORMAP ("Badge", bid, uid, badge);
	};

	// User Model
	struct UserModel
	{
		UserID uid;
		UserPwd pwd;
		size_t won;
		size_t los;

		ORMAP ("User", uid, pwd, won, los);
	};

	// Session
	struct SessionModel
	{
		UserID uid;
		RoomID rid;
		TimePoint heartbeat;
	};

	// Room
	struct RoomModel
	{
		struct Player
		{
			static constexpr size_t maxX = 300, maxY = 200;
			static constexpr size_t maxPlayerPerRoom = 3;

			bool isReady;

			size_t x, y;
			size_t vx, vy;
			Pokemon::TimeGap timeGap;

			PokemonModel pokemonModel;
			std::unique_ptr<Pokemon> pokemon;
		};

		std::unordered_map<UserID, Player> players;
		bool isOver = false;
		std::mutex mtx;
	};

	// Server
	class PokemonServer
	{
		using json = nlohmann::json;

	public:
		PokemonServer (unsigned short port)
		{
			using namespace BOT_ORM;

			// Shared Runtime Data
			std::unordered_map<SessionID, SessionModel> sessions;
			std::unordered_map<RoomID, RoomModel> rooms;

			ORMapper mapper ("Pokemon.db");
			UserModel userModel;
			BadgeModel badgeModel;
			PokemonModel pokemonModel;
			auto field = FieldExtractor {
				userModel, badgeModel, pokemonModel };

			// Init DB
			try
			{
				mapper.CreateTbl (userModel);
				std::cout << "Created User Table\n";
			}
			catch (const std::exception &ex)
			{ std::cerr << ex.what () << std::endl; }
			try
			{
				mapper.CreateTbl (
					badgeModel,
					Constraint::Reference (field (badgeModel.uid),
										   field (userModel.uid)));
				std::cout << "Created Badge Table\n";
			}
			catch (const std::exception &ex)
			{ std::cerr << ex.what () << std::endl; }
			try
			{
				mapper.CreateTbl (
					pokemonModel,
					Constraint::Reference (field (pokemonModel.uid),
										   field (userModel.uid)));
				std::cout << "Created Pokemon Table\n";
			}
			catch (const std::exception &ex)
			{ std::cerr << ex.what () << std::endl; }

			// Helper Functions

			auto keepSession = [&sessions] (const SessionID &sid)
			{
				auto p = sessions.find (sid);
				if (p == sessions.end ())
					throw std::runtime_error ("You haven't Login");
				(*p).second.heartbeat = TimePointHelper::TimeNow ();
			};

			auto leaveRoom = [&sessions, &rooms] (const SessionID &sid)
			{
				auto &rid = sessions[sid].rid;
				if (rid == RoomID {})
					return false;

				// Remove Player from Game
				rooms[rid].players.erase (sessions[sid].uid);

				// Clear Room if Empty
				if (rooms[rid].players.empty ())
					rooms.erase (rid);

				rid = RoomID {};
				return true;
			};

			auto clearSession = [&sessions, &leaveRoom] ()
			{
				static const auto timeGap = std::chrono::minutes { 1 };

				auto timeNow = TimePointHelper::TimeNow ();
				for (auto p = sessions.begin (); p != sessions.end ();)
					if (timeNow - p->second.heartbeat > timeGap)
					{
						leaveRoom (p->first);
						p = sessions.erase (p);
					}
					else
						++p;
			};

			auto pokemonToJson = [] (const PokemonModel &pokemon)
			{
				return json {
					{ "pid", pokemon.pid },
					{ "uid", pokemon.uid },
					{ "name", pokemon.name },
					{ "level", pokemon.level },
					{ "exppoint", pokemon.expPoint },
					{ "atk", pokemon.atk },
					{ "def", pokemon.def },
					{ "hp", pokemon.hp },
					{ "timegap", pokemon.timeGap }
				};
			};

			auto userToJson = [&] (const UserModel &user, bool isOnline)
			{
				json userj {
					{ "uid", user.uid },
					{ "online", isOnline },
					{ "wonrate", (double) user.won /
					std::max (1u, user.won + user.los) }
				};

				auto badges = mapper.Query (badgeModel)
					.Where (field (badgeModel.uid) == user.uid)
					.ToList ();

				for (const auto &badge : badges)
					userj["badges"].emplace_back (badge.badge);

				auto pokemons = mapper.Query (pokemonModel)
					.Where (field (pokemonModel.uid) == user.uid)
					.ToList ();

				for (const auto &pokemon : pokemons)
					userj["pokemons"].emplace_back (pokemonToJson (pokemon));

				return userj;
			};

#pragma region Accounting

			SetHandler ("register",
						[&] (json &response, const json &request)
			{
				const auto uid = request.at ("uid").get<UserID> ();
				const auto pwd = request.at ("pwd").get<UserPwd> ();

				if (uid.empty () || pwd.size () < 6)
					throw std::runtime_error ("Empty UID or Pwd's Size < 6");

				mapper.Transaction ([&] ()
				{
					try
					{
						mapper.Insert (UserModel { uid, pwd, 0, 0 });
					}
					catch (...)
					{
						throw std::runtime_error ("UserID has been Taken");
					}

					try
					{
						mapper.Insert (BadgeModel {
							0, uid, "Newcomer" }, false);
						mapper.Insert (BadgeModel {
							0, uid, "Welcome to Pokemon Game" }, false);
						mapper.Insert (BadgeModel {
							0, uid, "Hello World" }, false);
					}
					catch (...)
					{
						throw std::runtime_error ("Failed to Init Badge");
					}

					// Allocate Pokemon to User
					// Return true if Insert New Pokemon Successfully
					try
					{
						const auto initPokemonCount = 3;
						for (size_t i = 0; i < initPokemonCount; i++)
							mapper.Insert (
								*PokemonModel::FromPokemon (
									0, uid, *Pokemon::NewPokemon ()),
								false);
					}
					catch (...)
					{
						throw std::runtime_error ("Failed to Init Pokemon");
					}
				});
				response = "Registered successfully";
			});

			SetHandler ("login",
						[&] (json &response, const json &request)
			{
				const auto uid = request.at ("uid").get<UserID> ();
				const auto pwd = request.at ("pwd").get<UserPwd> ();

				auto users = mapper.Query (userModel)
					.Where (field (userModel.uid) == uid &&
							field (userModel.pwd) == pwd)
					.ToList ();

				if (users.empty ())
					throw std::runtime_error ("Bad Login Attempt");

				for (const auto &session : sessions)
					if (session.second.uid == uid)
					{
						sessions.erase (session.first);
						break;
					}

				auto timeNow = TimePointHelper::TimeNow ();
				auto sid = uid + TimePointHelper::ToStr (timeNow);
				sessions[sid] = SessionModel { uid, RoomID {}, timeNow };

				response = json
				{
					{ "sid", sid },
					{ "user", userToJson (users.front (), true) }
				};
			});

			SetHandler ("logout",
						[&] (json &response, const json &request)
			{
				const auto sid = request.at ("sid").get<SessionID> ();
				keepSession (sid);

				sessions.erase (sid);
				response = "Logout Successfully";
			});

#pragma endregion

#pragma region Query

			SetHandler ("pokemons",
						[&] (json &response, const json &request)
			{
				const auto sid = request.at ("sid").get<SessionID> ();
				keepSession (sid);

				auto pokemons = mapper.Query (pokemonModel)
					.ToList ();

				if (pokemons.empty ())
					throw std::runtime_error ("No Pokemon Found");

				for (const auto &pokemon : pokemons)
					response.emplace_back (pokemonToJson (pokemon));
			});

			SetHandler ("users",
						[&] (json &response, const json &request)
			{
				const auto sid = request.at ("sid").get<SessionID> ();
				keepSession (sid);

				auto users = mapper.Query (userModel)
					.ToList ();

				if (users.empty ())
					throw std::runtime_error ("No User Found");

				// Lazy loading:
				// Remove offine users
				clearSession ();

				std::unordered_map<UserID, bool> isOnline;
				for (const auto &session : sessions)
					isOnline[session.second.uid] = true;

				for (const auto &user : users)
					response.emplace_back (userToJson (user, isOnline[user.uid]));
			});

#pragma endregion

#pragma region Room

			SetHandler ("rooms",
						[&] (json &response, const json &request)
			{
				const auto sid = request.at ("sid").get<SessionID> ();
				keepSession (sid);

				if (rooms.empty ())
					throw std::runtime_error ("No Room Found");

				for (const auto &room : rooms)
					response.emplace_back (room.first);
			});

			SetHandler ("roomenter",
						[&] (json &response, const json &request)
			{
				const auto sid = request.at ("sid").get<SessionID> ();
				const auto rid = request.at ("rid").get<RoomID> ();
				const auto pid = request.at ("pid").get<PokemonID> ();
				keepSession (sid);

				if (rid == RoomID {})
					throw std::runtime_error ("Room ID must be NOT empty");

				auto &ridInSession = sessions[sid].rid;
				if (ridInSession != RoomID {})
					throw std::runtime_error ("Already in a Room");

				auto &room = rooms[rid];
				if (room.players.size () ==
					RoomModel::Player::maxPlayerPerRoom)
					throw std::runtime_error ("Room is Full");

				auto pokemons = mapper.Query (pokemonModel)
					.Where (
						field (pokemonModel.pid) == pid &&
						field (pokemonModel.uid) == sessions[sid].uid
					)
					.ToList ();

				// Remember to Check if this Pokemon belonging to this User
				if (pokemons.empty ())
					throw std::runtime_error ("It's NOT your Poekmon");

				auto pokemon = pokemons.front ().ToPokemon ();
				std::random_device rand;

				auto initX = rand () % (
					RoomModel::Player::maxX - pokemon->GetSize ().first);
				auto initY = rand () % (
					RoomModel::Player::maxY - pokemon->GetSize ().second);

				ridInSession = rid;
				room.players[sessions[sid].uid] = RoomModel::Player
				{
					false,
					initX, initY,
					0, 0,
					pokemon->GetTimeGap (),
					std::move (pokemons.front ()),
					std::move (pokemon)
				};

				response = json {
					{ "width", RoomModel::Player::maxX },
					{ "height", RoomModel::Player::maxY }
				};
			});

			SetHandler ("roomleave",
						[&] (json &response, const json &request)
			{
				const auto sid = request.at ("sid").get<SessionID> ();
				keepSession (sid);

				if (!leaveRoom (sid))
					throw std::runtime_error ("Not in a Room");

				response = "Left the Room";
			});

			SetHandler ("roomready",
						[&] (json &response, const json &request)
			{
				const auto sid = request.at ("sid").get<SessionID> ();
				const auto isReady = request.at ("ready").get<bool> ();
				keepSession (sid);

				const auto &rid = sessions[sid].rid;
				if (rid == RoomID {})
					throw std::runtime_error ("Not in a Room");

				rooms[rid].players[sessions[sid].uid].isReady = isReady;

				for (const auto &playerPair : rooms[rid].players)
				{
					const auto &player = playerPair.second;
					response.emplace_back (json {
						{ "uid", playerPair.first },
						{ "ready", player.isReady },
						{ "pokemon", pokemonToJson (player.pokemonModel) }
					});
				}
			});

#pragma endregion

#pragma region Gaming

			SetHandler ("gamesync",
						[&] (json &response, const json &request)
			{
				const auto sid = request.at ("sid").get<SessionID> ();
				const auto moveX = request.at ("movex").get<int> ();
				const auto moveY = request.at ("movey").get<int> ();
				const auto atkX = request.at ("atkx").get<int> ();
				const auto atkY = request.at ("atky").get<int> ();
				const auto isDef = request.at ("def").get<bool> ();
				keepSession (sid);

				const auto &rid = sessions[sid].rid;
				if (rid == RoomID {})
					throw std::runtime_error ("Not in a Room");

				// Make sure only one player manip. this room resource
				auto &room = rooms[rid];
				std::lock_guard<std::mutex> lg (room.mtx);

				auto isStart = true;
				auto playerAlive = room.players.size ();
				for (const auto &playerPair : room.players)
				{
					const auto &player = playerPair.second;
					if (!player.isReady)
						isStart = false;
					if (player.pokemon->GetCurHP () == 0)
						--playerAlive;
				}

				// Handle Action
				if (!room.isOver)
				{
					// Todo
				}

				// Game Not Started
				if (!isStart)
					throw std::runtime_error ("Not all players ready");

				// Game Over
				if (playerAlive <= 1)
				{
					// Handle Game Over only once
					if (!room.isOver && playerAlive > 0)
					{
						UserID winner;
						for (const auto &playerPair : room.players)
							if (playerPair.second.pokemon->GetCurHP () != 0)
								winner = playerPair.first;

						// Change Owners
						for (auto &playerPair : room.players)
						{
							auto &pokemon = playerPair.second.pokemonModel;
							pokemon.uid = winner;
							mapper.Update (pokemon);
						}
					}
					// Set Game Over
					room.isOver = true;
				}

				// Response Result
				if (room.isOver)
				{
					json resultplayersj;
					for (const auto &playerPair : room.players)
					{
						const auto &player = playerPair.second;
						resultplayersj.emplace_back (json {
							{ "uid", playerPair.first },
							{ "won", player.pokemon->GetCurHP () != 0 }
						});
					}

					response = {
						{ "over", true },
						{ "resultplayers", std::move (resultplayersj) }
					};
					return;
				}

				// Response Player States
				json gameplayersj;
				for (const auto &playerPair : room.players)
				{
					const auto &player = playerPair.second;
					gameplayersj.emplace_back (json {
						{ "uid", playerPair.first },
						{ "x", player.x }, { "y", player.y },
						{ "vx", player.vx }, { "vy", player.vy },
						{ "timegap", player.timeGap },
						{ "hp", player.pokemon->GetCurHP () }
					});
				}

				// Todo
				json gamedamagesj;

				response = {
					{ "over", false },
					{ "gameplayers", std::move (gameplayersj) },
					{ "gamedamages", std::move (gamedamagesj) }
				};
			});

#pragma endregion

			// Run
			BOT_Socket::Server (port, [&] (const std::string &request,
										   std::string &response)
			{
				try
				{
					const json req = json::parse (request.c_str ());
					json res { { "success", true } };
					_handlers.at (req.at ("request")) (
						res["response"], req.at ("param"));
					response = res.dump ();
				}
				catch (const std::logic_error &)
				{
					response = json {
						{ "success", false },
						{ "response", "Bad Request" }
					}.dump ();
				}
				catch (const std::runtime_error &ex)
				{
					response = json {
						{ "success", false },
						{ "response", ex.what () }
					}.dump ();
				}
			});
		}

	private:
		using Handler = std::function<
			void (json &response, const json &request)
		>;
		std::unordered_map<std::string, Handler> _handlers;

		inline void SetHandler (std::string event, Handler handler)
		{
			_handlers.emplace (std::move (event), std::move (handler));
		}
	};
}

#endif // !POKEMON_SERVER_H