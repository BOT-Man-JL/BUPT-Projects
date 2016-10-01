#ifndef POKEMON_SERVER_H
#define POKEMON_SERVER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <mutex>
#include <strstream>

#include "Pokemon.h"
#include "Socket.h"
#include "SQL.h"

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

	void GetFmtStr (
		std::string &fmt,
		std::pair<std::string, std::string> arg1,
		bool isPrimaryKey)
	{
		auto fmt1 = std::move (arg1.first) + ' ' + std::move (arg1.second);
		if (isPrimaryKey)
			fmt1 += " primary key";
		fmt = std::move (fmt1) + fmt;
	}

	template <typename... Args>
	void GetFmtStr (
		std::string &fmt,
		std::pair<std::string, std::string> arg1,
		bool isPrimaryKey,
		std::pair<std::string, std::string> arg2,
		Args... args)
	{
		fmt += "," + std::move (arg2.first) + " " + std::move (arg2.second);
		GetFmtStr (fmt, std::move (arg1), isPrimaryKey, args...);
	}

	template <typename T, typename Q>
	std::string SQLWhere (
		const std::pair<std::string, std::string> &field,
		const Q &op,
		const T &value)
	{
		static_assert (std::is_arithmetic<T>::value, "T must be arithmetic");
		return field.first + op + std::to_string (value);
	}

	template <typename Q>
	inline std::string SQLWhere (
		const std::pair<std::string, std::string> &field,
		const Q &op,
		const std::string &value)
	{
		return field.first + op + "'" + value + "'";
	}

	template <typename Q>
	inline std::string SQLWhere (
		const std::pair<std::string, std::string> &field,
		const Q &op,
		const char *value)
	{
		return SQLWhere (field, op, std::string (value));
	}

	template <typename T>
	std::string SQLInsert (const T &value)
	{
		static_assert (std::is_arithmetic<T>::value, "T must be arithmetic");
		return std::to_string (value);
	}

	inline std::string SQLInsert (const std::string &value)
	{
		return "'" + value + "'";
	}

	inline std::string SQLInsert (const char *value)
	{
		return SQLInsert (std::string (value));
	}

	template <typename T, typename... Args>
	std::string SQLInsert (const T &value,
						   Args... args)
	{
		static_assert (std::is_arithmetic<T>::value, "T must be arithmetic");
		return std::to_string (value) + "," + SQLInsert (args...);
	}

	template <typename... Args>
	inline std::string SQLInsert (const std::string &value,
								  Args... args)
	{
		return "'" + value + "'," + SQLInsert (args...);
	}

	template <typename... Args>
	inline std::string SQLInsert (const char *value,
								  Args... args)
	{
		return SQLInsert (std::string (value), args...);
	}

	inline std::string PokemonToString (
		int mid, const std::string &uid,
		const PokemonGame::Pokemon &pokemon)
	{
		return SQLInsert (
			mid, uid,
			pokemon.GetName (),
			pokemon.GetLevel (),
			pokemon.GetExp (),
			pokemon.GetAtk (),
			pokemon.GetDef (),
			pokemon.GetTimeGap (),
			pokemon.GetFullHP (),
			pokemon.GetHP ());
	}
}

namespace PokemonGame
{
	class PokemonServer
	{
	public:
		PokemonServer (unsigned short port)
		{
			using namespace PokemonGame_Impl;

			const auto dbName = "Pokemon.db";
			const auto tblUser = "UserTbl";
			const auto tblSession = "SessionTbl";
			const auto tblPokemon = "PokemonTbl";

			const std::pair<std::string, std::string>
				// Accounting
				fmtUid { "uid", "varchar(16)" },
				fmtPwd { "pwd", "varchar(16)" },
				fmtSid { "sid", "varchar(32)" },
				fmtMid { "mid", "int" },
				// User Attr
				fmtWon { "won", "int" },
				fmtLos { "los", "int" },
				fmtBdg { "bdg", "varchar(1024)" },
				// Pokemon Attr
				fmtMNam { "mnam", "varchar(16)" },
				fmtMLev { "mlev", "int" },
				fmtMExp { "mexp", "int" },
				fmtMAtk { "matk", "int" },
				fmtMDef { "mdef", "int" },
				fmtMGap { "mgap", "int" },
				fmtMFHp { "mfhp", "int" },
				fmtMCHp { "mchp", "int" };

			// Init db
			{
				SQLConnector connector (dbName);
				auto CreateTable = [&] (const std::string &tbl,
										const std::string &fmt)
				{
					try
					{
						connector.CreateTable (tbl, fmt);
						std::cout << "Created " + tbl << std::endl;
					}
					catch (const std::exception &e)
					{
						std::cerr << e.what () << std::endl;
					}
				};

				std::string strFmt;
				GetFmtStr (strFmt, fmtUid, true, fmtPwd,
						   fmtWon, fmtLos, fmtBdg);
				CreateTable (tblUser, strFmt);

				strFmt.clear ();
				GetFmtStr (strFmt, fmtUid, true, fmtSid);
				CreateTable (tblSession, strFmt);

				strFmt.clear ();
				GetFmtStr (strFmt, fmtMid, true, fmtUid,
						   fmtMNam, fmtMLev, fmtMExp,
						   fmtMAtk, fmtMDef, fmtMGap,
						   fmtMFHp, fmtMCHp);
				CreateTable (tblPokemon, strFmt);
			}

			auto CheckArgAndLogin = [&] (std::string &response,
										 const std::vector<std::string> &args,
										 size_t argCount,
										 bool isLoginRequired)
			{
				if (args.size () < argCount)
				{
					SetResponse (response, false, "Too Few Arguments");
					return false;
				}

				if (isLoginRequired)
				{
					if (args.size () < 2)
					{
						SetResponse (response, false, "Server Error");
						return false;
					}

					const auto &sid = args[1];
					try
					{
						using namespace PokemonGame_Impl;

						auto isLogin = false;
						SQLConnector connector (dbName);
						auto whereStr = SQLWhere (fmtSid, "=", sid);
						connector.QueryValue (tblSession, fmtUid.first, whereStr,
											  [&] (int, char **argv, char **)
						{ isLogin = true; });
						if (isLogin)
							return true;
					}
					catch (const std::exception&) {}

					SetResponse (response, false, "You haven't Login");
					return false;
				}
				return true;
			};

			// Allocate Pokemon to User
			// Return true if Insert New Pokemon Successfully
			auto AddPokemon = [&] (const std::string &uid,
								   const Pokemon &pokemon)
			{
				try
				{
					SQLConnector connector (dbName);
					auto mid =
						connector.CountValue (tblPokemon, fmtMid.first);
					auto sqlParam = PokemonToString (mid, uid, pokemon);
					connector.InsertValue (tblPokemon, sqlParam);
					return true;
				}
				catch (const std::exception&) {}
				return false;
			};

			// Handle Login
			// Succeed if Set Sid / Get Sid is OK
			// Fail if No Uid Found / Wrong Pwd / Get Sid Failed
			SetHandler ("Login", [&] (std::string &response,
									  const std::vector<std::string> &args)
			{
				if (!CheckArgAndLogin (response, args, 3, false))
					return;

				const auto &uid = args[1];
				const auto &pwd = args[2];
				SetResponse (response, false, "Bad Login Attempt");
				try
				{
					SQLConnector connector (dbName);
					auto whereStr = SQLWhere (fmtUid, "=", uid);
					connector.QueryValue (tblUser, fmtPwd.first, whereStr,
										  [&] (int, char **argv, char **)
					{
						// Wrong PWD
						if (argv[0] != pwd) return;
						try
						{
							// Set new SID
							auto sid = uid + std::to_string (clock ());
							sid = sid.substr (0, 32);
							connector.InsertValue (tblSession,
												   SQLInsert (uid, sid));
							SetResponse (response, true, sid);
						}
						catch (const std::exception&)
						{
							// Get SID From UID
							auto whereStr = SQLWhere (fmtUid, "=", uid);
							connector.QueryValue (tblSession, fmtSid.first, whereStr,
												  [&] (int, char **argv, char **)
							{
								auto sid = argv[0];
								SetResponse (response, true, sid);
							});
						}
					});
				}
				catch (const std::exception&) {}
			});

			// Handle Register
			// Succeed if Set Uid and New Pokemon are OK
			// Fail if Set Uid / New Pokemon Failed
			SetHandler ("Register", [&] (std::string &response,
										 const std::vector<std::string> &args)
			{
				if (!CheckArgAndLogin (response, args, 3, false))
					return;

				const auto &uid = args[1];
				const auto &pwd = args[2];
				SetResponse (response, false, "UserID has been Taken");
				try
				{
					SQLConnector connector (dbName);
					connector.InsertValue (tblUser,
										   SQLInsert (
											   uid, pwd,
											   1, 0,
											   "New Commer\n"
											   "Welcome to Pokemon Game\n"
											   "Hello World"
										   ));

					// Seed 3 Init Pokemon
					const auto initPokemonCount = 3;
					auto isAddPokemon = true;
					for (size_t i = 0; i < initPokemonCount; i++)
					{
						auto newPokemon =
							NewPokemonRandly ();
						if (!AddPokemon (uid, *newPokemon))
							isAddPokemon = false;
						delete newPokemon;
					}

					if (isAddPokemon)
						SetResponse (response, true,
									 "Registered successfully");
					else
						SetResponse (response, false,
									 "Registered successfully but "
									 "Allocate Pokemon Failed :-(");
				}
				catch (const std::exception&) {}
			});

			// Handle Logout
			// Succeed if Delete Sid is OK
			// Fail if Delete Sid Failed
			SetHandler ("Logout", [&] (std::string &response,
									   const std::vector<std::string> &args)
			{
				if (!CheckArgAndLogin (response, args, 2, true))
					return;

				const auto &sid = args[1];
				SetResponse (response, false, "Logout Failed");
				try
				{
					SQLConnector connector (dbName);
					auto whereStr = SQLWhere (fmtSid, "=", sid);
					connector.DeleteValue (tblSession, whereStr);
					SetResponse (response, true, "Logout Successfully");
				}
				catch (const std::exception&) {}
			});

			// Handle UsersPokemons
			// Succeed if Pokemon Found
			// Fail if No Sid Found / No Pokemon Found / Query Failed
			SetHandler ("UsersPokemons", [&] (std::string &response,
											  const std::vector<std::string> &args)
			{
				if (!CheckArgAndLogin (response, args, 3, true))
					return;

				const auto &uidToSearch = args[2];
				SetResponse (response, false, "No Pokemon Found");
				try
				{
					SQLConnector connector (dbName);
					std::string ret;
					auto whereStr = SQLWhere (fmtUid, "=", uidToSearch);
					connector.QueryValue (tblPokemon, fmtMNam.first, whereStr,
										  [&] (int, char **argv, char **)
					{
						ret += argv[0];
						ret += '\n';
					});
					ret.pop_back ();
					if (ret.size ())
						SetResponse (response, true, ret);
				}
				catch (const std::exception&) {}
			});

			// Handle UsersWonRate
			// Succeed if User Found
			// Fail if No Sid Found / No User Found / Query Failed
			SetHandler ("UsersWonRate", [&] (std::string &response,
											 const std::vector<std::string> &args)
			{
				if (!CheckArgAndLogin (response, args, 3, true))
					return;

				const auto &uidToSearch = args[2];
				SetResponse (response, false, "No Such User");
				try
				{
					SQLConnector connector (dbName);
					auto whereStr = SQLWhere (fmtUid, "=", uidToSearch);
					connector.QueryValue (tblUser,
										  fmtWon.first + "," + fmtLos.first,
										  whereStr,
										  [&] (int, char **argv, char **)
					{
						std::strstream strs;
						auto cWon = 0, cLos = 0;
						strs << argv[0] << " " << argv[1];
						strs >> cWon >> cLos;
						auto ret = std::to_string (
							(double) cWon / (cWon + cLos));
						SetResponse (response, true, ret);
					});
				}
				catch (const std::exception&) {}
			});

			// Handle UsersBadges
			// Succeed if Badges Found
			// Fail if No Sid Found / No User Found / Query Failed
			SetHandler ("UsersBadges", [&] (std::string &response,
											const std::vector<std::string> &args)
			{
				if (!CheckArgAndLogin (response, args, 3, true))
					return;

				const auto &uidToSearch = args[2];
				SetResponse (response, false, "No Badge Found");
				try
				{
					SQLConnector connector (dbName);
					auto whereStr = SQLWhere (fmtUid, "=", uidToSearch);
					connector.QueryValue (tblUser, fmtBdg.first, whereStr,
										  [&] (int, char **argv, char **)
					{
						SetResponse (response, true, argv[0]);
					});
				}
				catch (const std::exception&) {}
			});

			// Handle UsersAll
			// Succeed if User Found
			// Fail if No Sid Found / No User Found / Query Failed
			SetHandler ("UsersAll", [&] (std::string &response,
										 const std::vector<std::string> &args)
			{
				if (!CheckArgAndLogin (response, args, 2, true))
					return;

				SetResponse (response, false, "No User Registered");
				try
				{
					SQLConnector connector (dbName);
					std::string ret;
					connector.QueryValue (tblUser, fmtUid.first, "",
										  [&] (int, char **argv, char **)
					{
						ret += argv[0];
						ret += '\n';
					});
					ret.pop_back ();
					if (ret.size ())
						SetResponse (response, true, ret);
				}
				catch (const std::exception&) {}
			});

			// Handle UsersOnline
			// Succeed if User Found
			// Fail if No Sid Found / No User Found / Query Failed
			SetHandler ("UsersOnline", [&] (std::string &response,
											const std::vector<std::string> &args)
			{
				if (!CheckArgAndLogin (response, args, 2, true))
					return;

				SetResponse (response, false, "No User Online");
				try
				{
					SQLConnector connector (dbName);
					std::string ret;
					connector.QueryValue (tblSession, fmtUid.first, "",
										  [&] (int, char **argv, char **)
					{
						ret += argv[0];
						ret += '\n';
					});
					ret.pop_back ();
					if (ret.size ())
						SetResponse (response, true, ret);
				}
				catch (const std::exception&) {}
			});

			// Run
			std::mutex ioLock;
			Server (port, [&] (const std::string &request,
							   std::string &response)
			{
				auto args = SplitStr (request, "\n");
				if (args.size () < 1)
					SetResponse (response, false, "No Arguments");
				else if (_handlers.find (args[0]) == _handlers.end ())
					SetResponse (response, false, "No Handler Found");
				else
					_handlers[args[0]] (response, args);

				std::lock_guard<std::mutex> lck (ioLock);
				std::cout << '\n' << request << '\n' << response << "\n\n";
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

#endif // !POKEMON_SERVER_H