#ifndef POKEMON_CLIENT_H
#define POKEMON_CLIENT_H

#include <string>
#include <vector>
#include <unordered_map>

#include "Socket.h"
#include "Pokemon.h"
#include "Shared.h"

namespace PokemonGame
{
	using Pokemons = std::unordered_map<
		PokemonID,
		std::unique_ptr<Pokemon>
	>;
	using Players = std::vector<Player>;

	class PokemonClient
	{
	public:
		PokemonClient (const std::string &ipAddr,
					   unsigned short port)
			: _sockClient (ipAddr, port)
		{}

		const std::string &ErrMsg () const
		{
			return _errMsg;
		}

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

		Pokemons &MyPokemons ()
		{
			return _myPokemons;
		}

		bool UsersPokemons (const std::string &uid,
							Pokemons &out)
		{
			auto response = Request ("UsersPokemons", _sessionID, uid);
			return HandleResponse (response, [&] ()
			{
				for (const auto &id : response)
					out[(PokemonID) std::stoull (id)]
					= PokemonFromID (id);
			});
		}

		bool PokemonAll (Pokemons &out)
		{
			auto response = Request ("PokemonAll", _sessionID);
			return HandleResponse (response, [&] ()
			{
				for (const auto &id : response)
					out[(PokemonID) std::stoull (id)]
					= PokemonFromID (id);
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
						PokemonID pokemon1,
						PokemonID pokemon2,
						PokemonID pokemon3)
		{
			auto response = Request ("RoomEnter", _sessionID, roomId,
									 std::to_string (pokemon1),
									 std::to_string (pokemon2),
									 std::to_string (pokemon3));
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
			return HandleResponse<3> (response, [&] ()
			{
				_tEnterRoom = PokemonGame_Impl::TimePointFromStr (response[0]);
				posX = (size_t) std::stoull (response[1]);
				posY = (size_t) std::stoull (response[2]);
				response.erase (response.begin (), response.begin () + 3);

				_players.clear ();
				while (response.size () >= 7)
				{
					_players.emplace_back (PlayerFromResponse (response));
					response.erase (response.begin (), response.begin () + 7);
				}
			});
		}

		Players &ViewPlayers ()
		{
			return _players;
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

		std::unique_ptr<Pokemon> PokemonFromID (const std::string &id)
		{
			auto response = Request ("PokemonInfo", _sessionID, id);
			if (!HandleResponse<8> (response))
				throw std::runtime_error (
					_errMsg + ": Loading Pokemon Info Failed");

			return std::unique_ptr<Pokemon> (
				Pokemon::NewPokemon (response[0],
									 std::stoul (response[1]),
									 std::stoul (response[2]),
									 std::stoul (response[3]),
									 std::stoul (response[4]),
									 std::stoul (response[5]),
									 std::stoul (response[6]),
									 std::stoul (response[7]))
				);
		}

		Player PlayerFromResponse (
			const std::vector<std::string> &response)
		{
			using namespace PokemonGame_Impl;

			std::vector<std::string> pids
			{
				response[4],
				response[5],
				response[6]
			};
			const auto defaultPid = std::to_string (PokemonID ());

			PokemonsOfPlayer pokemons;
			for (const auto &pid : pids)
				pokemons.emplace_back (
				(PokemonID) std::stoull (pid),
					pid != defaultPid ? PokemonFromID (pid) : nullptr);

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

		BOT_Socket::Client _sockClient;
		std::string _errMsg;

		UserID _userID;
		SessionID _sessionID;
		Pokemons _myPokemons;

		// Room
		PokemonGame_Impl::TimePoint _tEnterRoom;
		size_t posX, posY;
		Players _players;
	};

}

#endif // !POKEMON_CLIENT_H