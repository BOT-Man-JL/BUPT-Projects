#ifndef POKEMON_SQL_H
#define POKEMON_SQL_H

#include <string>
#include <functional>
#include <exception>
#include "SQLite\sqlite3.h"

namespace PokemonGame_Impl
{
	class SQLConnector
	{
	public:
		SQLConnector (const std::string &fileName)
		{
			auto rc = sqlite3_open (fileName.c_str (), &db);
			if (rc)
			{
				sqlite3_close (db);
				throw std::runtime_error (std::string ("Can't open database: %s\n")
										  + sqlite3_errmsg (db));
			}
		}

		void Excute (const std::string &cmd,
					 int callback(void *, int argc, char **argv, char **azColName))
		{
			char *zErrMsg = 0;
			auto rc = sqlite3_exec (db, cmd.c_str(), callback, 0, &zErrMsg);
			if (rc != SQLITE_OK)
			{
				auto errStr = std::string ("SQL error: ") + zErrMsg;
				sqlite3_free (zErrMsg);
				throw std::runtime_error (errStr);
			}
		}

		~SQLConnector ()
		{
			sqlite3_close (db);
		}

	private:
		sqlite3 *db;
	};
}

#endif // !POKEMON_SQL_H