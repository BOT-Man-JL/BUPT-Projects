#ifndef POKEMON_CLIENT_H
#define POKEMON_CLIENT_H

#include <string>
#include <vector>
#include <array>
#include <unordered_map>

#include "Socket.h"
#include "Pokemon.h"
#include "Shared.h"

namespace PokemonGame
{
	using PokemonsWithID = std::unordered_map<
		PokemonID,
		std::unique_ptr<Pokemon>
	>;

	class PokemonClient
	{
	public:
		PokemonClient (const std::string &ipAddr,
					   unsigned short port)
			: _sockClient (ipAddr, port)
		{}

		const std::string &ErrMsg () const
		{ return _errMsg; }

		const PokemonGame::UserID &MyUID () const
		{ return _userID; }

		const PokemonsWithID &MyPokemons () const
		{ return _myPokemons; }

		const std::unordered_map<UserID, Player> &ViewPlayers () const
		{ return _players; }

#pragma region Accounting

		bool Login (const std::string &uid,
					const std::string &pwd)
		{
			auto response = Request ("Login", uid, pwd);
			return HandleResponse<1> (response, [&] ()
			{
				_sessionID = response[0];
				_userID = uid;
				UsersPokemons (_userID, _myPokemons);
			});
		}

		bool Register (const std::string &uid,
					   const std::string &pwd)
		{
			auto response = Request ("Register", uid, pwd);
			return HandleResponse (response);
		}

		bool Logout ()
		{
			auto response = Request ("Logout", _sessionID);
			return HandleResponse (response);
		}

#pragma endregion

#pragma region User Info

		bool UsersWonRate (const std::string &uid,
						   double &out)
		{
			auto response = Request ("UsersWonRate", _sessionID, uid);
			return HandleResponse<1> (response, [&] ()
			{
				out = std::stod (response[0]);
			});
		}

		bool UsersBadges (const std::string &uid,
						  std::vector<std::string> &out)
		{
			auto response = Request ("UsersBadges", _sessionID, uid);
			return HandleResponse (response, [&] ()
			{
				out = std::move (response);
			});
		}

		bool UsersAll (std::vector<std::string> &out)
		{
			auto response = Request ("UsersAll", _sessionID);
			return HandleResponse (response, [&] ()
			{
				out = std::move (response);
			});
		}

		bool UsersOnline (std::vector<std::string> &out)
		{
			auto response = Request ("UsersOnline", _sessionID);
			return HandleResponse (response, [&] ()
			{
				out = std::move (response);
			});
		}

#pragma endregion

#pragma region Pokemon Info

		bool UsersPokemons (const std::string &uid,
							PokemonsWithID &out)
		{
			auto response = Request ("UsersPokemons", _sessionID, uid);
			return HandleResponse (response, [&] ()
			{
				for (const auto &id : response)
					out[(PokemonID) std::stoull (id)] =
					std::unique_ptr<Pokemon> (PokemonFromID (id));
			});
		}

		bool PokemonAll (PokemonsWithID &out)
		{
			auto response = Request ("PokemonAll", _sessionID);
			return HandleResponse (response, [&] ()
			{
				for (const auto &id : response)
					out[(PokemonID) std::stoull (id)] =
					std::unique_ptr<Pokemon> (PokemonFromID (id));
			});
		}

#pragma endregion

#pragma region Room

		bool RoomQuery (std::vector<std::string> &out)
		{
			auto response = Request ("RoomQuery", _sessionID);
			return HandleResponse (response, [&] ()
			{
				out = std::move (response);
			});
		}

		bool RoomEnter (const std::string &roomId,
						const std::array<PokemonID, 3> &pokemonIds)
		{
			auto response = Request ("RoomEnter", _sessionID, roomId,
									 std::to_string (pokemonIds[0]),
									 std::to_string (pokemonIds[1]),
									 std::to_string (pokemonIds[2]));
			return HandleResponse (response);
		}

		bool RoomLeave ()
		{
			auto response = Request ("RoomLeave", _sessionID);
			return HandleResponse (response);
		}

		bool RoomReady ()
		{
			auto response = Request ("RoomReady", _sessionID);
			return HandleResponse (response);
		}

		bool RoomState ()
		{
			auto response = Request ("RoomState", _sessionID);
			if ((response.size () - 2) % 7)
			{
				_errMsg = "Invalid Response size";
				return false;
			}

			return HandleResponse<8> (response, [&] ()
			{
				_tSync = PokemonGame_Impl::TimePointFromStr (response[0]);
				_tLocal = std::chrono::system_clock::now ();
				response.erase (response.begin ());

				_players.clear ();
				while (!response.empty ())
				{
					_players[response[0]] = PlayerFromResponse (response);
					response.erase (response.begin (), response.begin () + 7);
				}
			});
		}

#pragma endregion

#pragma region Gaming

		bool Lockstep (Player::Action action)
		{
			using namespace PokemonGame_Impl;

			auto actionStr = ActionToStr (action);
			action.timestamp =
				(std::chrono::system_clock::now () - _tLocal) + _tSync;

			auto response = Request ("Lockstep", _sessionID, actionStr);
			if ((response.size () - 2) % 2)
			{
				_errMsg = "Invalid Response size";
				return false;
			}

			return HandleResponse<1> (response, [&] ()
			{
				_tSync = TimePointFromStr (response[0]);
				_tLocal = std::chrono::system_clock::now ();
				response.erase (response.begin ());

				while (!response.empty ())
				{
					_players[response[0]].actions.emplace_back (
						ActionFromStr (response[1]));
					response.erase (response.begin (), response.begin () + 2);
				}

				_players[_userID].actions.emplace_back (std::move (action));
				LockstepUpdate ();
			});
		}

#pragma endregion

	private:
		std::vector<std::string> Request (const std::string &strToken)
		{
			try
			{
				return PokemonGame_Impl::SplitStr (
					_sockClient.Request (strToken), "\n");
			}
			catch (const std::exception &ex)
			{
				return { "0", ex.what () };
			}
		}

		template<typename... Args>
		inline std::vector<std::string> Request (const std::string &strToken,
												 const std::string &arg1,
												 Args & ... args)
		{
			return Request (strToken + '\n' + arg1, args...);
		}

		template<size_t responseCount = 0>
		bool HandleResponse (std::vector<std::string> &response,
							 std::function<void ()> fnCaseSucceeded
							 = std::function<void ()> ())
		{
			if (response[0] == "0")
			{
				if (response.size () >= 2)
					_errMsg = response[1];
				else
					_errMsg = "Error without a message";
				return false;
			}

			response.erase (response.begin ());
			if (response.size () < responseCount)
			{
				_errMsg = "Too few Response Arguments";
				return false;
			}

			try
			{
				if (fnCaseSucceeded) fnCaseSucceeded ();
				return true;
			}
			catch (const std::exception &ex)
			{
				_errMsg = ex.what ();
				return false;
			}
		}

		Pokemon* PokemonFromID (const std::string &id)
		{
			auto response = Request ("PokemonInfo", _sessionID, id);
			if (!HandleResponse<8> (response))
				throw std::runtime_error (
					_errMsg + ": Loading Pokemon Info Failed");

			return Pokemon::NewPokemon (
				response[0],
				std::stoul (response[1]),
				std::stoul (response[2]),
				std::stoul (response[3]),
				std::stoul (response[4]),
				std::stoul (response[5]),
				std::stoul (response[6]),
				std::stoul (response[7]));
		}

		Player PlayerFromResponse (
			const std::vector<std::string> &response)
		{
			using namespace PokemonGame_Impl;

			std::vector<std::string> pidStrs
			{
				response[4], response[5], response[6]
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
					std::unique_ptr<Pokemon> (PokemonFromID (pidStrs[i]))
				);
			}

			return Player
			{
				// Uid
				response[0],
				// Is Ready
				response[1] == "0" ? false : true,
				// Position
				(size_t) std::stoull (response[2]),
				(size_t) std::stoull (response[3]),
				// Pokemon
				std::move (pokemons)
			};
		}

		void LockstepUpdate ()
		{
			for (auto &playerPair : _players)
			{
				auto &player = playerPair.second;
				auto &pokemon = *(player.pokemons.front ().second);
				const auto &action = player.actions.back ();

				const auto maxX = Player::maxX;
				const auto maxY = Player::maxY;

				switch (action.action)
				{
				case ActionType::None:
					// Do nothing
					break;

				case ActionType::Move:
					switch (action.param)
					{
					case MoveDir::A:
						if (player.x > 0) player.x--;
						break;
					case MoveDir::D:
						if (player.x < maxX) player.x++;
						break;
					case MoveDir::W:
						if (player.y > 0) player.y--;
						break;
					case MoveDir::S:
						if (player.y < maxY) player.y++;
						break;
					default:
						throw std::runtime_error ("Wrong Param");
						break;
					}
					break;

				case ActionType::Attack:
					break;
				case ActionType::Defend:
					break;
				case ActionType::Recover:
					break;
				case ActionType::Switch:
					break;

				default:
					throw std::runtime_error ("WTF? (Will not hit)");
					break;
				}
			}
		}

		BOT_Socket::Client _sockClient;
		std::string _errMsg;

		UserID _userID;
		SessionID _sessionID;
		PokemonsWithID _myPokemons;

		// Gaming
		TimePoint _tSync, _tLocal;
		std::unordered_map<UserID, Player> _players;
	};

}

#endif // !POKEMON_CLIENT_H