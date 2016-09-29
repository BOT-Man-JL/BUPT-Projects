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
					 int callback (void *, int argc, char **argv, char **azColName))
		{
			char *zErrMsg = 0;
			auto rc = sqlite3_exec (db, cmd.c_str (), callback, 0, &zErrMsg);
			if (rc != SQLITE_OK)
			{
				auto errStr = std::string ("SQL error: ") + zErrMsg;
				sqlite3_free (zErrMsg);
				throw std::runtime_error (errStr);
			}
		}

		void CreateTable (const std::string &tableName,
						  const std::string &params)
		{
			auto cmd = "create table " + tableName + " (" + params + ");";
			Excute (cmd, _callback);
		}

		void DropTable (const std::string &tableName)
		{
			auto cmd = "drop table " + tableName + ";";
			Excute (cmd, _callback);
		}

		void InsertValue (const std::string &tableName,
						  const std::string &value)
		{
			auto cmd = "insert into " + tableName + " values (" + value + ");";
			Excute (cmd, _callback);
		}

		void DeleteValue (const std::string &tableName,
						  const std::string &where)
		{
			std::string cmd;
			if (where.size ())
				cmd = "delete from " + tableName + " where " + where + ";";
			else
				cmd = "delete * from " + tableName + ";";
			Excute (cmd, _callback);
		}

		void QueryValue (const std::string &tableName,
						 const std::string &select,
						 const std::string &where,
						 int callback (void *, int argc, char **argv, char **azColName))
		{
			std::string cmd;
			if (where.size ())
				cmd = "select " + select + " from " + tableName + " where " + where + ";";
			else
				cmd = "select " + select + " from " + tableName + ";";
			Excute (cmd, callback);
		}

		~SQLConnector ()
		{
			sqlite3_close (db);
		}

	private:
		sqlite3 *db;
		static int _callback (void *, int argc, char **argv, char **azColName) { return 0; }
	};
}

#endif // !POKEMON_SQL_H