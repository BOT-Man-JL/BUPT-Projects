#ifndef POKEMON_PROTOCOL_H
#define POKEMON_PROTOCOL_H

/*
# Pokemon Protocol

PokemonServer is the Server who handle Request from Client;
PokemonClient is the Client who send Request to Server;

## Format

### Request Format

>> Token
>> Param
>> \0

### Response Format

>> Is Succeeded (0/1)
>> Param
>> \0

## Request Token

Tokens specify the types of a Request.

- Register
- Login
- Logout

## Request and Response Param

### Login / Register

>> User ID
>> User Password
>>
>> Session ID (succeeded) / Msg (failed)

### Logout

>> Session ID
>>
>> Msg

### ...

*/

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

#include "Pokemon.h"
#include "Socket.h"
#include "SQL.h"

namespace PokemonGame
{
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

	class PokemonClient
	{
	public:
		PokemonClient (const std::string &ipAddr,
					   unsigned short port)
			: _sockClient (ipAddr, port)
		{}

		~PokemonClient ()
		{}

		const std::string &ErrMsg () const
		{
			return _errMsg;
		}

		bool Login (const std::string &uid,
					const std::string &pwd)
		{
			auto response = Request (Token::Login,
									 uid + '\n' + pwd);
			if (response.size () >= 2)
			{
				if (response[0] == "1")
				{
					_sessionID = response[1];
					return true;
				}
				else
					_errMsg = response[1];
			}
			else
				_errMsg = "Bad Response";
			return false;
		}

		bool Register (const std::string &uid,
					   const std::string &pwd)
		{
			auto response = Request (Token::Register,
									 uid + '\n' + pwd);
			if (response.size () >= 2)
			{
				if (response[0] == "1")
				{
					_sessionID = response[1];
					return true;
				}
				else
					_errMsg = response[1];
			}
			else
				_errMsg = "Bad Response";
			return false;
		}

		bool Logout ()
		{
			auto response = Request (Token::Logout,
									 _sessionID);
			if (response.size () >= 1 && response[0] == "1")
			{
				_sessionID = "";
				return true;
			}

			if (response.size () >= 2)
				_errMsg = response[1];
			else
				_errMsg = "Bad Response";
			return false;
		}

		std::string ViewMyWonRate ()
		{
			// Todo
		}

		std::vector<std::string> ViewMyTokens ()
		{
			// Todo
		}

		std::vector<std::pair<std::string, std::string>> ViewAllUsers ()
		{
			// Todo
		}

		std::vector<std::string> ViewOnlineUsers ()
		{
			// Todo
		}

		std::string ViewUsersWonRate (const std::string &uid)
		{
			// Todo
		}

		std::vector<std::string> ViewUsersTokens (const std::string &uid)
		{
			// Todo
		}

	private:
		enum class Token
		{
			Login,
			Register,
			Logout
		};

		std::vector<std::string> Request (Token token,
										  const std::string &reqStr)
		{
			std::string strToken;
			switch (token)
			{
			case Token::Login:
				strToken = "Login";
				break;

			case Token::Register:
				strToken = "Register";
				break;

			case Token::Logout:
				strToken = "Logout";
				break;

			default:
				break;
			}
			return SplitStr (
				_sockClient.Request (strToken + '\n' + reqStr), "\n");
		}

		PokemonGame_Impl::Client _sockClient;
		std::string _sessionID;
		std::string _errMsg;
	};

	class PokemonServer
	{
	public:
		PokemonServer (unsigned short port)
		{
			const auto strUserTbl = "UserTbl";
			const auto strPokemonTbl = "PokemonTbl";

			const auto strUserTblFmt = "uid varchar(16) primary key, pwd varchar(16)";
			const auto strPokemonTblFmt =
				"id varchar(16) primary key, name varchar(16), master varchar(16)";

			// Init db
			{
				PokemonGame_Impl::SQLConnector connector ("Pokemon.db");
				try
				{
					connector.CreateTable (strUserTbl, strUserTblFmt);
					std::cout << "Created User Table\n";
				}
				catch (const std::exception &e)
				{
					std::cerr << e.what () << std::endl;
				}
				try
				{
					connector.CreateTable (strPokemonTbl, strPokemonTblFmt);
					std::cout << "Created Pokemon Table\n";
				}
				catch (const std::exception &e)
				{
					std::cerr << e.what () << std::endl;
				}
			}

			// Set Login
			SetHandler ("Login", [&] (std::string &response,
									  const std::vector<std::string>
									  &cmds)
			{
				if (cmds.size () < 3)
				{
					SetResponse (response, false, "Too Few Arguments");
					return;
				}

				SetResponse (response, false, "Login Failed");
				try
				{
					PokemonGame_Impl::SQLConnector connector ("Pokemon.db");
					connector.QueryValue (strUserTbl,
										  "pwd", "uid='" + cmds[1] + '\'',
										  [&] (int argc, char **argv, char **azColName)
					{
						if (argc > 0 && argv[0] == cmds[2])
							SetResponse (response, true, cmds[1] + cmds[2]);
						return 0;
					});
				}
				catch (const std::exception&)
				{}
			});

			// Set Register
			SetHandler ("Register", [&] (std::string &response,
										 const std::vector<std::string>
										 &cmds)
			{
				if (cmds.size () < 3)
				{
					SetResponse (response, false, "Too Few Arguments");
					return;
				}

				SetResponse (response, false, "Register Failed");
				try
				{
					PokemonGame_Impl::SQLConnector connector ("Pokemon.db");
					connector.InsertValue (strUserTbl, '\'' + cmds[1] + "','"
										   + cmds[2] + '\'');
					SetResponse (response, true, cmds[1] + cmds[2]);
				}
				catch (const std::exception&)
				{}
			});

			// Set Logout
			SetHandler ("Logout", [&] (std::string &response,
										 const std::vector<std::string>
										 &cmds)
			{
				if (cmds.size () < 2)
				{
					SetResponse (response, false, "Too Few Arguments");
					return;
				}

				SetResponse (response, false, "Logout Successfully");
			});

			// Run
			PokemonGame_Impl::Server (port,
									  [&] (const std::string &request,
										   std::string &response)
			{
				auto cmds = SplitStr (request, "\n");
				if (cmds.size () < 1)
					SetResponse (response, false, "No Arguments");
				else if (_handlers.find (cmds[0]) == _handlers.end ())
					SetResponse (response, false, "No Handler Found");
				else
					_handlers[cmds[0]] (response, cmds);
			});
		}

	private:
		static void SetResponse (std::string &response,
								 bool isSucceeded,
								 const std::string &param)
		{
			response = (isSucceeded ? "1\n" : "0\n") + param;
		}

		void SetHandler (std::string cmd,
						 std::function<void (std::string &response,
											 const std::vector<std::string>
											 &fullCmd)> handler)
		{
			_handlers.emplace (std::make_pair (std::move (cmd), std::move (handler)));
		}

		std::unordered_map<std::string,
			std::function<void (std::string &,
								const std::vector<std::string> &)>>
			_handlers;
	};

}

#endif // !POKEMON_PROTOCOL_H