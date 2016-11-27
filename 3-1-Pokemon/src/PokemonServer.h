#ifndef POKEMON_SERVER_H
#define POKEMON_SERVER_H

#include <vector>
#include <string>
#include <tuple>
#include <unordered_map>
#include <iostream>
#include <memory>
#include <chrono>

#include "Socket.h"
#include "Pokemon.h"
#include "Shared.h"
#include "Model.h"

namespace PokemonGame
{
	class PokemonServer
	{
	public:
		PokemonServer (unsigned short port)
		{
			using namespace PokemonGame_Impl;
			using namespace BOT_ORM;

			// Shared Runtime Data
			Sessions sessions;
			Rooms rooms;

			ORMapper mapper ("Pokemon.db");
			PokemonModel pokemonModel;
			UserModel userModel;
			auto field = FieldExtractor { pokemonModel, userModel };

			// Init DB
			try
			{
				mapper.CreateTbl (PokemonModel {});
				std::cout << "Created Pokemon Table\n";
			}
			catch (const std::exception &ex)
			{
				std::cerr << ex.what () << std::endl;
			}
			try
			{
				mapper.CreateTbl (UserModel {});
				std::cout << "Created User Table\n";
			}
			catch (const std::exception &ex)
			{
				std::cerr << ex.what () << std::endl;
			}

			auto getPokemonModelById = [&] (const PokemonID &id)
			{
				auto pokemons = mapper.Query (pokemonModel)
					.Where (field (pokemonModel.id) == id)
					.ToVector ();

				if (pokemons.empty ())
					return std::unique_ptr<PokemonModel> (nullptr);

				return std::make_unique<PokemonModel> (pokemons[0]);
			};

#pragma region Accounting

			// Handle Register
			// Succeed if Set Uid and New Pokemon are OK
			// Fail if Set Uid / New Pokemon Failed
			SetHandler<2, false> ("Register",
								  [&] (std::string &response,
									   const std::vector<std::string> &args)
			{
				const auto &uid = args[0];
				const auto &pwd = args[1];

				if (uid.empty () || pwd.size () < 6)
				{
					SetResponse (response, false, "Empty UID or Pwd's Size < 6");
					return;
				}

				try
				{
					mapper.Insert (UserModel { uid, pwd, 1, 0,
								   "Newcomer\n"
								   "Welcome to Pokemon Game\n"
								   "Hello World" });
				}
				catch (const std::exception &)
				{
					SetResponse (response, false, "UserID has been Taken");
					return;
				}

				// Allocate Pokemon to User
				// Return true if Insert New Pokemon Successfully
				auto AddPokemon = [&] (const std::string &uid,
									   const Pokemon &pokemon)
				{
					static PokemonModel _pm;
					try
					{
						// Auto Set ID
						auto newPkm = std::unique_ptr<PokemonModel> (
							PokemonModel::NewFromPokemon (0, uid, pokemon));
						mapper.Insert (*newPkm, false);
						return true;
					}
					catch (const std::exception &)
					{
						return false;
					}
				};

				// Seed 3 Init Pokemon
				const auto initPokemonCount = 3;
				auto isAddPokemon = true;
				for (size_t i = 0; i < initPokemonCount; i++)
				{
					auto newPokemon =
						std::unique_ptr<Pokemon> (Pokemon::NewPokemon ());
					if (!AddPokemon (uid, *newPokemon))
						isAddPokemon = false;
				}

				if (isAddPokemon)
					SetResponse (response, true,
								 "Registered successfully");
				else
					SetResponse (response, false,
								 "Registered successfully but "
								 "Allocate Pokemon Failed :-(");
			});

			// Handle Login
			// Succeed if Set Sid / Get Sid is OK
			// Fail if No Uid Found / Wrong Pwd / Get Sid Failed
			SetHandler<2, false> ("Login",
								  [&] (std::string &response,
									   const std::vector<std::string> &args)
			{
				auto userId = args[0];

				// No User & Password Match
				if (mapper.Query (userModel)
					.Where (field (userModel.uid) == userId &&
							field (userModel.pwd) == args[1])
					.Select (Expression::Count ()) == 0)
				{
					SetResponse (response, false, "Bad Login Attempt");
					return;
				}

				// Login Already
				for (const auto &session : sessions)
					if (session.second.uid == userId)
					{
						//SetResponse (response, true, session.first);
						SetResponse (response, false, "Login already");
						return;
					}

				// New Session
				auto sid = userId +
					TimePointToStr (std::chrono::system_clock::now ());
				sessions[sid] = SessionModel { userId, RoomID {} };
				SetResponse (response, true, sid);
			});

			// Handle Logout
			// Succeed if Delete Sid is OK
			// Fail if Delete Sid Failed
			SetHandler<1> ("Logout",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				const auto &sid = args[0];
				sessions.erase (sid);
				SetResponse (response, true, "Logout Successfully");
			});

#pragma endregion

#pragma region User Info

			// Handle UsersWonRate
			// Succeed if User Found
			// Fail if No User Found / Select Failed
			SetHandler<2> ("UsersWonRate",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				auto users = mapper.Query (userModel)
					.Where (field (userModel.uid) == args[1])
					.ToVector ();

				if (users.empty ())
					SetResponse (response, false, "No Such User");
				else
					SetResponse (response, true,
								 std::to_string (
								 (double) users[0].won
									 / (users[0].won + users[0].los)));
			});

			// Handle UsersBadges
			// Succeed if Badges Found
			// Fail if No User Found / Select Failed
			SetHandler<2> ("UsersBadges",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				auto users = mapper.Query (userModel)
					.Where (field (userModel.uid) == args[1])
					.ToVector ();

				if (users.empty ())
					SetResponse (response, false, "No Badge Found");
				else
					SetResponse (response, true, users[0].badge);
			});

			// Handle UsersAll
			// Succeed if User Found
			// Fail if No User Found / Select Failed
			SetHandler<1> ("UsersAll",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				auto users = mapper.Query (userModel)
					.ToList ();

				if (users.empty ())
				{
					SetResponse (response, false, "No User Registered");
					return;
				}

				std::string ret;
				for (auto & user : users)
				{
					ret += std::move (user.uid);
					ret += '\n';
				}
				ret.pop_back ();
				SetResponse (response, true, ret);
			});

			// Handle UsersOnline
			// Succeed if User Found
			// Fail if No User Found
			SetHandler<1> ("UsersOnline",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				if (sessions.empty ())
				{
					SetResponse (response, false, "No User Online");
					return;
				}

				std::string ret;
				for (auto & session : sessions)
				{
					ret += std::move (session.second.uid);
					ret += '\n';
				}
				ret.pop_back ();
				SetResponse (response, true, ret);
			});

#pragma endregion

#pragma region Pokemon Info

			// Handle PokemonInfo
			// Succeed if Pokemon Found
			// Fail if No Pokemon Found / Select Failed
			SetHandler<2> ("PokemonInfo",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				const auto &pokemonId = (PokemonID) std::stoull (args[1]);
				auto pm = getPokemonModelById (pokemonId);

				if (pm == nullptr)
				{
					SetResponse (response, false, "No such Pokemon");
					return;
				}

				SetResponse (response, true,
							 pm->name,
							 std::to_string (pm->level),
							 std::to_string (pm->expPoint),
							 std::to_string (pm->atk),
							 std::to_string (pm->def),
							 std::to_string (pm->hp),
							 std::to_string (pm->fullHP),
							 std::to_string (pm->timeGap));
			});

			// Handle UsersPokemons
			// Succeed if Pokemon Found
			// Fail if No Pokemon Found / Select Failed
			SetHandler<2> ("UsersPokemons",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				auto pokemons = mapper.Query (pokemonModel)
					.Where (field (pokemonModel.uid) == args[1])
					.ToList ();

				if (pokemons.empty ())
				{
					SetResponse (response, false, "No Pokemon Found");
					return;
				}

				std::string ret;
				for (const auto &pokemon : pokemons)
				{
					ret += std::to_string (pokemon.id);
					ret += '\n';
				}
				ret.pop_back ();
				SetResponse (response, true, ret);
			});

			// Handle PokemonAll
			// Succeed if Pokemon Found
			// Fail if No Pokemon Found / Select Failed
			SetHandler<1> ("PokemonAll",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				auto pokemons = mapper.Query (pokemonModel)
					.ToList ();

				if (pokemons.empty ())
				{
					SetResponse (response, false, "No Pokemon Found");
					return;
				}

				std::string ret;
				for (const auto &pokemon : pokemons)
				{
					ret += std::to_string (pokemon.id);
					ret += '\n';
				}
				ret.pop_back ();
				SetResponse (response, true, ret);
			});

#pragma endregion

#pragma region Room

			// Handle RoomQuery
			// Succeed if Any Room
			// Fail if Not any Room
			SetHandler<1> ("RoomQuery",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				if (rooms.empty ())
				{
					SetResponse (response, false, "No Room");
					return;
				}

				std::string ret;
				for (auto & room : rooms)
				{
					ret += std::move (room.first);
					ret += '\n';
				}
				ret.pop_back ();
				SetResponse (response, true, ret);
			});

			// Handle RoomEnter
			// Succeed if Room is not full
			// Fail if Room is full or already in this Room
			SetHandler<5> ("RoomEnter",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				const auto &sid = args[0];
				const auto &rid = args[1];

				if (rid.empty ())
				{
					SetResponse (response, false, "Empty Room ID");
					return;
				}

				auto &room = rooms[rid];

				if (room.players.size () == RoomModel::maxPlayerPerRoom)
				{
					SetResponse (response, false, "Room is Full");
					return;
				}

				if (sessions[sid].rid != RoomID ())
				{
					SetResponse (response, false, "Already in a Room");
					return;
				}

				std::random_device rand;
				auto initX = rand () % Player::maxX;
				auto initY = rand () % Player::maxY;

				std::vector<std::string> pidStrs
				{
					args[2], args[3], args[4]
				};
				std::vector<PokemonID> pids;
				for (const auto &pidStr : pidStrs)
					pids.emplace_back ((PokemonID) std::stoull (pidStr));

				PokemonsOfPlayer pokemons;
				for (size_t i = 0; i < pids.size (); i++)
				{
					if (pids[i] == PokemonID ())
						continue;

					pokemons.emplace_back (
						pids[i],
						std::unique_ptr<Pokemon> (
							getPokemonModelById (pids[i])->ToPokemon ())
					);
				}

				room.players[sessions[sid].uid] = Player
				{
					false, initX, initY, std::move (pokemons)
				};
				sessions[sid].rid = rid;
				SetResponse (response, true, "Entered this Room");
			});

			// Handle RoomLeave
			// Succeed if in a Room
			// Fail if Not in a Room
			SetHandler<1> ("RoomLeave",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				const auto &sid = args[0];

				if (sessions[sid].rid == RoomID ())
				{
					SetResponse (response, false, "Not in a Room");
					return;
				}

				auto &rid = sessions[sid].rid;
				rooms[rid].players.erase (sessions[sid].uid);

				// Empty Room
				if (rooms[rid].players.empty ())
					rooms.erase (rid);

				rid = RoomID ();
				SetResponse (response, true, "Left the Room");
			});

			// Handle RoomReady
			// Succeed if in a Room
			// Fail if Not in a Room
			SetHandler<1> ("RoomReady",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				const auto &sid = args[0];

				if (sessions[sid].rid == RoomID ())
				{
					SetResponse (response, false, "Not in a Room");
					return;
				}

				auto &room = rooms[sessions[sid].rid];
				auto &you = room.players[sessions[sid].uid];

				// Set me Ready
				you.isReady = !you.isReady;

				SetResponse (response, true, "You are Ready Now ~");
			});

			// Handle RoomState
			// Succeed if in a Room
			// Fail if Not in a Room
			SetHandler<1> ("RoomState",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				const auto &sid = args[0];

				if (sessions[sid].rid == RoomID ())
				{
					SetResponse (response, false, "Not in a Room");
					return;
				}

				const auto &room = rooms[sessions[sid].rid];

				std::string ret =
					TimePointToStr (std::chrono::system_clock::now ()) + "\n";
				for (const auto &player : room.players)
				{
					ret += player.first + "\n"
						+ (player.second.isReady ? "1\n" : "0\n")
						+ std::to_string (player.second.x) + "\n"
						+ std::to_string (player.second.y) + "\n";

					for (const auto &pokemon : player.second.pokemons)
						ret += std::to_string (pokemon.first) + "\n";
				}
				ret.pop_back ();
				SetResponse (response, true, ret);
			});

#pragma endregion

#pragma region Gaming

			// Handle Lockstep
			SetHandler<2> ("Lockstep",
						   [&] (std::string &response,
								const std::vector<std::string> &args)
			{
				const auto timeSlot = std::chrono::milliseconds (50);

				const auto &sid = args[0];
				if (sessions[sid].rid == RoomID ())
				{
					SetResponse (response, false, "Not in a Room");
					return;
				}

				const auto &uid = sessions[sid].uid;
				const auto action = ActionFromStr (args[1]);
				auto &room = rooms[sessions[sid].rid];

				std::string ret =
					TimePointToStr (std::chrono::system_clock::now ()) + "\n";
				{
					// Sync Action Queue
					std::lock_guard<std::mutex> lg (room.mtxSync);

					// Push your action to ohters' queue
					for (auto &actionQueue : room.actionQueues)
					{
						if (actionQueue.first == uid)
							continue;
						actionQueue.second.push (action);
					}

					// Pop your queue
					auto &yourQueue = room.actionQueues[uid];
					while (!yourQueue.empty ())
					{
						ret += ActionToStr (yourQueue.top ());
						yourQueue.pop ();
						ret += "\n";
					}
				}
				ret.pop_back ();
				SetResponse (response, true, ret);
			});

#pragma endregion

			// Run
			BOT_Socket::Server (port, [&] (const std::string &request,
										   std::string &response)
			{
				auto args = SplitStr (request, "\n");
				if (args.size () < 1)
				{
					SetResponse (response, false, "No Arguments");
					return;
				}

				const auto event = std::move (args.front ());
				args.erase (args.begin ());

				// Find Handler
				if (_handlers.find (event) == _handlers.end ())
				{
					SetResponse (response, false, "No Handler Found");
					return;
				}

				const auto &handler = _handlers[event];
				const auto expectedArgs = std::get <0> (handler);
				const auto isLoginRequired = std::get <1> (handler);
				const auto &handlerFn = std::get <2> (handler);

				// Check Login
				if (isLoginRequired &&
					sessions.find (args[0]) == sessions.end ())
				{
					SetResponse (response, false, "You haven't Login");
					return;
				}

				// Check Args Count
				if (args.size () < expectedArgs)
				{
					SetResponse (response, false, "Too Few Arguments");
					return;
				}

				// Delegated to Handler
				handlerFn (response, args);
			});
		}

	private:
		inline static void SetResponse (std::string &response,
										bool isSucceeded,
										std::string arg1)
		{
			response = (isSucceeded ? "1\n" : "0\n") + std::move (arg1);
		}

		template<typename... Args>
		inline static void SetResponse (std::string &response,
										bool isSucceeded,
										std::string arg1,
										std::string arg2,
										Args & ... args)
		{
			return SetResponse (response, isSucceeded,
								std::move (arg1) + "\n" + std::move (arg2),
								args...);
		}

		using Handler = std::function<
			void (std::string &response,
				  const std::vector<std::string> &args)
		>;

		// Event -> <expectedArgs, isLoginRequired, Handler>
		std::unordered_map<std::string,
			std::tuple<size_t, bool, Handler>
		> _handlers;

		template <size_t expectedArgs = 0, bool isLoginRequired = true>
		void SetHandler (std::string event, Handler handler)
		{
			static_assert (!isLoginRequired || expectedArgs > 0,
						   "Session ID is the first Argument of Request");
			_handlers.emplace (std::move (event),
							   std::make_tuple (expectedArgs,
												isLoginRequired,
												std::move (handler)));
		}
	};

}

#endif // !POKEMON_SERVER_H