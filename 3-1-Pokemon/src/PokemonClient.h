#ifndef POKEMON_CLIENT_H
#define POKEMON_CLIENT_H

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <sstream>
#include <iomanip>

#include "Socket.h"
#include "Pokemon.h"
#include "Shared.h"

namespace PokemonGame
{
	using PokemonsWithID = std::unordered_map<
		PokemonID,
		std::unique_ptr<Pokemon>
	>;

	void PrintPokemon (std::ostream &os,
					   const Pokemon *pokemon)
	{
		os << "\t" << std::setw (10)
			<< pokemon->GetName ()
			<< ": [LV:"
			<< pokemon->GetLevel ()
			<< ", Exp:"
			<< pokemon->GetExp ()
			<< ", Atk:"
			<< pokemon->GetAtk ()
			<< ", Def:"
			<< pokemon->GetDef ()
			<< ", HP:"
			<< pokemon->GetHP ()
			<< ", Gap:"
			<< pokemon->GetTimeGap ()
			<< "]\n";
	}

	void PrintPlayer (std::ostream &os,
					  const UserID &uid,
					  const Player &player)
	{
		os << "\t" << uid << ", "
			<< std::boolalpha << player.isReady << ", "
			<< player.x << ", "
			<< player.y << "\n";

		PrintPokemon (os, player.pokemon.get ());
	}

	class PokemonClient
	{
	public:
		PokemonClient (const std::string &ipAddr,
					   unsigned short port)
			: _sockClient (ipAddr, port)
		{}

		const std::string &ErrMsg () const
		{ return _errMsg; }

		const UserID &MyUID () const
		{ return _userID; }

		const PokemonsWithID &MyPokemons () const
		{ return _myPokemons; }

		// Using Physics to Update Players' States
		Players &GetPlayers ()
		{ return _players; }

		// Using Physics to Consume Actions
		ActionQueue &GetActionQueue ()
		{ return _actionQueue; }

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
						PokemonID pokemonId)
		{
			auto response = Request ("RoomEnter", _sessionID, roomId,
									 std::to_string (pokemonId));
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
			constexpr auto segSize = 6;

			auto response = Request ("RoomState", _sessionID);
			if ((response.size () - 2) % segSize)
			{
				_errMsg = "Invalid Response size";
				return false;
			}

			return HandleResponse<segSize> (response, [&] ()
			{
				_tSync = PokemonGame_Impl::TimePointFromStr (response[0]);
				_tLocal = std::chrono::system_clock::now ();
				response.erase (response.begin ());

				_players.clear ();
				while (!response.empty ())
				{
					_players[response[0]] = Player
					{
						// Is Ready
						response[1] == "0" ? false : true,
						// Position
						(size_t) std::stoull (response[2]),
						(size_t) std::stoull (response[3]),
						(Pokemon::TimeGap) std::stoull (response[4]),
						// Pokemon
						(PokemonID) std::stoull (response[5]),
						std::unique_ptr<Pokemon> (PokemonFromID (response[5]))
					};;
					response.erase (response.begin (), response.begin () + 6);
				}
			});
		}

#pragma endregion

#pragma region Gaming

		bool Lockstep (Action action)
		{
			using namespace PokemonGame_Impl;

			action.uid = _userID;
			action.timestamp =
				(std::chrono::system_clock::now () - _tLocal) + _tSync;

			auto response = Request ("Lockstep", _sessionID, ActionToStr (action));
			_actionQueue.push (std::move (action));

			return HandleResponse<1> (response, [&] ()
			{
				_tSync = TimePointFromStr (response[0]);
				_tLocal = std::chrono::system_clock::now ();
				response.erase (response.begin ());

				for (const auto &responseArg : response)
					_actionQueue.push (ActionFromStr (responseArg));
			});
		}

#pragma endregion

	private:
		std::vector<std::string> Request (const std::string &strToken)
		{
			try
			{
				auto response = _sockClient.Request (strToken);
				//std::cout << response << std::endl;
				return PokemonGame_Impl::SplitStr (std::move (response), "\n");
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
			if (!HandleResponse<7> (response))
				throw std::runtime_error (
					_errMsg + ": Loading Pokemon Info Failed");

			return Pokemon::NewPokemon (
				response[0],
				std::stoul (response[1]),
				std::stoul (response[2]),
				std::stoul (response[3]),
				std::stoul (response[4]),
				std::stoul (response[5]),
				std::stoul (response[6]));
		}

		BOT_Socket::Client _sockClient;
		std::string _errMsg;

		UserID _userID;
		SessionID _sessionID;
		PokemonsWithID _myPokemons;

		// Gaming
		TimePoint _tSync, _tLocal;
		Players _players;
		ActionQueue _actionQueue;
	};

}

#endif // !POKEMON_CLIENT_H