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
				auto tblUserFmt =
					fmtUid.first + ' ' + fmtUid.second + " primary key, " +
					fmtPwd.first + ' ' + fmtPwd.second + ", " +
					fmtWrt.first + ' ' + fmtWrt.second;
				auto tblSessionFmt =
					fmtUid.first + ' ' + fmtUid.second + " primary key, " +
					fmtSid.first + ' ' + fmtSid.second;
				auto tblPokemonFmt = 
					fmtMid.first + ' ' + fmtMid.second + " primary key, " +
					fmtUid.first + ' ' + fmtUid.second + ", " +
					fmtMNam.first + ' ' + fmtMNam.second + ", " +
					fmtMLev.first + ' ' + fmtMLev.second + ", " +
					fmtMExp.first + ' ' + fmtMExp.second + ", " +
					fmtMAtk.first + ' ' + fmtMAtk.second + ", " +
					fmtMDef.first + ' ' + fmtMDef.second + ", " +
					fmtMGap.first + ' ' + fmtMGap.second + ", " +
					fmtMFHp.first + ' ' + fmtMFHp.second + ", " +
					fmtMCHp.first + ' ' + fmtMCHp.second;

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
				CreateTable (tblUser, tblUserFmt);
				CreateTable (tblSession, tblSessionFmt);
				CreateTable (tblPokemon, tblPokemonFmt);
			}

			// Check Sid
			// Get UID from SID
			auto CheckSid = [&] (const std::string &sid,
								 std::string &uid)
			{
				auto ret = false;
				PokemonGame_Impl::SQLConnector connector (dbName);
				try
				{
					// Query Session
					connector.QueryValue (tblSession, fmtUid.first,
										  fmtSid.first + "='" + sid + '\'',
										  [&] (int, char **argv, char **)
					{
						// Found
						uid = argv[0];
						ret = true;
					});
				}
				catch (const std::exception&) {}

				return ret;
			};

			auto PokemonToString = [] (const Pokemon &pokemon)
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
			};

			auto StringToPokemon = [] (const std::string &str)
			{
				// Todo
			};

			auto AddPokemon = [&] (const std::string &uid,
								   const Pokemon &pokemon)
			{
				PokemonGame_Impl::SQLConnector connector (dbName);

				auto midIndex = connector.CountValue (tblPokemon, "mid", "");
				try
				{
					auto sqlParam = std::to_string (midIndex) +
						",'" + uid + "'," + PokemonToString (pokemon);
					connector.InsertValue (tblPokemon, sqlParam);
				}
				catch (const std::exception&)
				{
					return false;
				}
				return true;
			};

			// Handle Login
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
				PokemonGame_Impl::SQLConnector connector (dbName);
				try
				{
					// Query User
					connector.QueryValue (tblUser, fmtPwd.first,
										  fmtUid.first + "='" + uid + '\'',
										  [&] (int, char **argv, char **)
					{
						// Wrong PWD
						if (argv[0] != pwd)
							return;

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
							try
							{
								connector.QueryValue (tblSession, fmtSid.first,
													  fmtUid.first + "='" + uid + '\'',
													  [&] (int, char **argv, char **)
								{
									auto sid = argv[0];
									SetResponse (response, true, sid);
								});
							}
							catch (const std::exception&) {}
						}
					});
				}
				catch (const std::exception&) {}
			});

			// Handle Register
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
				PokemonGame_Impl::SQLConnector connector (dbName);
				try
				{
					connector.InsertValue (tblUser, '\'' + uid + "','"
										   + pwd + "',0");

					// Seed 3 Init Pokemon
					const auto initPokemonCount = 3;
					auto isAddPokemon = true;
					for (size_t i = 0; i < initPokemonCount; i++)
					{
						auto newPokemon = NewPokemonRandly ();
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
				PokemonGame_Impl::SQLConnector connector (dbName);
				try
				{
					connector.DeleteValue (tblSession,
										   fmtSid.first + "='" + sid + '\'');
					SetResponse (response, true, "Logout Successfully");
				}
				catch (const std::exception&) {}
			});

			// Handle UsersPokemons
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
				PokemonGame_Impl::SQLConnector connector (dbName);
				try
				{
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
				PokemonGame_Impl::SQLConnector connector (dbName);
				try
				{
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
				PokemonGame_Impl::SQLConnector connector (dbName);
				try
				{
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
				PokemonGame_Impl::SQLConnector connector (dbName);
				try
				{
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
				auto args = SplitStr (request, "\n");
				if (args.size () < 1)
					SetResponse (response, false, "No Arguments");
				else if (_handlers.find (args[0]) == _handlers.end ())
					SetResponse (response, false, "No Handler Found");
				else
					_handlers[args[0]] (response, args);

				{
					std::lock_guard<std::mutex> lck (ioLock);
					std::cout << '\n' << request << '\n' << response << "\n\n";
				}
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