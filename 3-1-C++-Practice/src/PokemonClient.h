#ifndef POKEMON_CLIENT_H
#define POKEMON_CLIENT_H

#include <string>
#include <vector>

#include "Pokemon.h"
#include "Socket.h"

namespace PokemonGame_Impl
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
}

namespace PokemonGame
{
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

		bool Login (const std::string &uid,
					const std::string &pwd)
		{
			auto response = Request ("Login", uid, pwd);
			return HandleResponse<2> (response, [&] ()
			{
				_sessionID = response[1];
				return true;
			});
		}

		bool Register (const std::string &uid,
					   const std::string &pwd)
		{
			auto response = Request ("Register", uid, pwd);
			return HandleResponse<2> (response, [&] ()
			{
				return true;
			});
		}

		bool Logout ()
		{
			auto response = Request ("Logout", _sessionID);
			return HandleResponse<2> (response, [&] ()
			{
				return true;
			});
		}

		bool UsersPokemons (const std::string &uid,
							std::vector<std::string> &out)
		{
			auto response = Request ("UsersPokemons", _sessionID, uid);
			return HandleResponse<2> (response, [&] ()
			{
				response.erase (response.begin ());
				out = std::move (response);
				return true;
			});
		}

		bool UsersWonRate (const std::string &uid,
						   double &out)
		{
			auto response = Request ("UsersWonRate", _sessionID, uid);
			return HandleResponse<2> (response, [&] ()
			{
				out = std::stod (response[1]);
				return true;
			});
		}

		bool UsersBadges (const std::string &uid,
						  std::vector<std::string> &out)
		{
			auto response = Request ("UsersBadges", _sessionID, uid);
			return HandleResponse<2> (response, [&] ()
			{
				response.erase (response.begin ());
				out = std::move (response);
				return true;
			});
		}

		bool UsersAll (std::vector<std::string> &out)
		{
			auto response = Request ("UsersAll", _sessionID);
			return HandleResponse<2> (response, [&] ()
			{
				response.erase (response.begin ());
				out = std::move (response);
				return true;
			});
		}

		bool UsersOnline (std::vector<std::string> &out)
		{
			auto response = Request ("UsersOnline", _sessionID);
			return HandleResponse<2> (response, [&] ()
			{
				response.erase (response.begin ());
				out = std::move (response);
				return true;
			});
		}

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

		template<typename T, typename... Args>
		std::vector<std::string> Request (const std::string &strToken,
										  const T &arg1, Args & ... args)
		{
			return Request (strToken + '\n' + arg1, args...);
		}

		template<size_t responseCount>
		bool HandleResponse (const std::vector<std::string> &response,
							 std::function<bool ()> fnCaseSucceeded)
		{
			static_assert (responseCount >= 2, "responseCount >= 2");

			if (response.size () < responseCount)
			{
				_errMsg = "Bad Response";
				return false;
			}
			else if (response[0] == "1")
				return fnCaseSucceeded ();
			else
			{
				_errMsg = response[1];
				return false;
			}
		}

		BOT_Socket::Client _sockClient;
		std::string _sessionID;
		std::string _errMsg;
	};

}

#endif // !POKEMON_CLIENT_H