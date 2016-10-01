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

	std::string PokemonToString (const PokemonGame::Pokemon &pokemon)
	{
		std::strstream strs;
		strs << '\'' << pokemon.GetName () << "',"
			<< pokemon.GetLevel () << ','
			<< pokemon.GetExp () << ','
			<< pokemon.GetAtk () << ','
			<< pokemon.GetDef () << ','
			<< pokemon.GetTimeGap () << ','
			<< pokemon.GetFullHP () << ','
			<< pokemon.GetHP ();
		std::string ret;
		strs >> ret;
		return std::move (ret);
	}
}

namespace PokemonGame
{
	class PokemonServer
	{
	public:
		PokemonServer (unsigned short port)
		{
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
				fmtWrt { "wrt", "decimal" },
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
				using namespace PokemonGame_Impl;

				PokemonGame_Impl::SQLConnector connector (dbName);
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
				GetFmtStr (strFmt, fmtUid, true, fmtPwd, fmtWrt);
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

			// Check Sid
			// Return true if Found, and Set uid
			auto CheckSid = [&] (const std::string &sid,
								 std::string &uid)
			{
				auto ret = false;
				try
				{
					PokemonGame_Impl::SQLConnector connector (dbName);
					// Get UID from SID
					connector.QueryValue (tblSession, fmtUid.first,
										  fmtSid.first + "='" + sid + '\'',
										  [&] (int, char **argv, char **)
					{
						uid = argv[0];
						ret = true;
					});
				}
				catch (const std::exception&) {}
				return ret;
			};

			// Allocate Pokemon to User
			// Return true if Insert New Pokemon Successfully
			auto AddPokemon = [&] (const std::string &uid,
								   const Pokemon &pokemon)
			{
				try
				{
					PokemonGame_Impl::SQLConnector connector (dbName);
					auto sqlParam = std::to_string (
						connector.CountValue (tblPokemon, "mid", "")) +
						",'" + uid + "'," +
						PokemonGame_Impl::PokemonToString (pokemon);
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
				if (args.size () < 3)
				{
					SetResponse (response, false, "Too Few Arguments");
					return;
				}

				const auto &uid = args[1];
				const auto &pwd = args[2];
				SetResponse (response, false, "Bad Login Attempt");

				try
				{
					PokemonGame_Impl::SQLConnector connector (dbName);
					connector.QueryValue (tblUser, fmtPwd.first,
										  fmtUid.first + "='" + uid + '\'',
										  [&] (int, char **argv, char **)
					{
						// Wrong PWD
						if (argv[0] != pwd) return;
						try
						{
							// Set new SID
							auto sid = uid + std::to_string (clock ());
							sid = sid.substr (0, 32);
							connector.InsertValue (tblSession, '\'' + uid +
												   "','" + sid + '\'');
							SetResponse (response, true, sid);
						}
						catch (const std::exception&)
						{
							// Get SID From UID
							connector.QueryValue (tblSession, fmtSid.first,
												  fmtUid.first + "='" + uid + '\'',
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
				if (args.size () < 3)
				{
					SetResponse (response, false, "Too Few Arguments");
					return;
				}

				const auto &uid = args[1];
				const auto &pwd = args[2];
				SetResponse (response, false, "UserID has been Taken");

				try
				{
					PokemonGame_Impl::SQLConnector connector (dbName);
					connector.InsertValue (tblUser, '\'' + uid + "','"
										   + pwd + "',0");

					// Seed 3 Init Pokemon
					const auto initPokemonCount = 3;
					auto isAddPokemon = true;
					for (size_t i = 0; i < initPokemonCount; i++)
					{
						auto newPokemon =
							PokemonGame_Impl::NewPokemonRandly ();
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
				if (args.size () < 2)
				{
					SetResponse (response, false, "Too Few Arguments");
					return;
				}

				const auto &sid = args[1];
				SetResponse (response, false, "Logout Failed");

				try
				{
					PokemonGame_Impl::SQLConnector connector (dbName);
					connector.DeleteValue (tblSession,
										   fmtSid.first + "='" + sid + '\'');
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
				if (args.size () < 3)
				{
					SetResponse (response, false, "Too Few Arguments");
					return;
				}

				const auto &sid = args[1];
				const auto &uidToSearch = args[2];

				if (!CheckSid (sid, std::string ()))
				{
					SetResponse (response, false, "You haven't Login");
					return;
				}

				SetResponse (response, false, "No Pokemon Found");
				try
				{
					PokemonGame_Impl::SQLConnector connector (dbName);
					std::string ret;
					auto where = fmtUid.first + "='" + uidToSearch + '\'';
					connector.QueryValue (tblPokemon, fmtMNam.first, where,
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
				if (args.size () < 3)
				{
					SetResponse (response, false, "Too Few Arguments");
					return;
				}

				const auto &sid = args[1];
				const auto &uidToSearch = args[2];

				if (!CheckSid (sid, std::string ()))
				{
					SetResponse (response, false, "You haven't Login");
					return;
				}

				SetResponse (response, false, "No Such User");
				try
				{
					PokemonGame_Impl::SQLConnector connector (dbName);
					auto where = fmtUid.first + "='" + uidToSearch + '\'';
					connector.QueryValue (tblUser, fmtWrt.first, where,
										  [&] (int, char **argv, char **)
					{
						auto ret = argv[0];
						SetResponse (response, true, ret);
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
				if (args.size () < 2)
				{
					SetResponse (response, false, "Too Few Arguments");
					return;
				}

				const auto &sid = args[1];

				if (!CheckSid (sid, std::string ()))
				{
					SetResponse (response, false, "You haven't Login");
					return;
				}

				SetResponse (response, false, "No User Registered");
				try
				{
					PokemonGame_Impl::SQLConnector connector (dbName);
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
				if (args.size () < 2)
				{
					SetResponse (response, false, "Too Few Arguments");
					return;
				}

				const auto &sid = args[1];

				if (!CheckSid (sid, std::string ()))
				{
					SetResponse (response, false, "You haven't Login");
					return;
				}

				SetResponse (response, false, "No User Online");
				try
				{
					PokemonGame_Impl::SQLConnector connector (dbName);
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
			PokemonGame_Impl::Server (port,
									  [&] (const std::string &request,
										   std::string &response)
			{
				auto args = PokemonGame_Impl::SplitStr (request, "\n");
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