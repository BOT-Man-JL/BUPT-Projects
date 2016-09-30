#ifndef POKEMON_SQL_H
#define POKEMON_SQL_H

#include <string>
#include <functional>
#include <thread>
#include <exception>
#include <strstream>
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
				throw std::runtime_error (
					std::string ("Can't open database: %s\n")
					+ sqlite3_errmsg (db));
			}
		}

		// Don't throw in callback
		void Excute (const std::string &cmd,
					 std::function<void (int argc, char **argv,
										 char **azColName) noexcept>
					 callback)
		{
			const size_t MAX_TRIAL = 16;
			char *zErrMsg = 0;
			int rc;

			auto callbackWrapper = [] (void *fn, int argc, char **argv,
									   char **azColName)
			{
				static_cast<std::function
					<void (int argc,
						   char **argv,
						   char **azColName) noexcept> *>
						   (fn)->operator()(argc, argv, azColName);
				return 0;
			};

			for (size_t iTry = 0; iTry < MAX_TRIAL; iTry++)
			{
				rc = sqlite3_exec (db, cmd.c_str (), callbackWrapper,
								   &callback, &zErrMsg);
				if (rc != SQLITE_BUSY)
					break;

				using namespace std::chrono_literals;
				std::this_thread::sleep_for (20ms);
			}

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
			auto cmd = "create table " + tableName +
				" (" + params + ");";
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
			auto cmd = "insert into " + tableName +
				" values (" + value + ");";
			Excute (cmd, _callback);
		}

		void DeleteValue (const std::string &tableName,
						  const std::string &where)
		{
			std::string cmd;
			if (where.size ())
				cmd = "delete from " + tableName +
				" where (" + where + ");";
			else
				cmd = "delete * from " + tableName + ";";
			Excute (cmd, _callback);
		}

		size_t CountValue (const std::string &tableName,
						   const std::string &select,
						   const std::string &where)
		{
			std::string cmd;
			if (where.size ())
				cmd = "select count (" + select + ") as num from " + tableName +
				" where (" + where + ");";
			else
				cmd = "select count (" + select + ") as num from " + tableName + ";";

			auto ret = 0;
			auto callback = [&] (int argc, char **argv,
								char **azColName)
			{
				std::istrstream strs (argv[0]);
				strs >> ret;
			};
			Excute (cmd, callback);
			return ret;
		}

		void QueryValue (const std::string &tableName,
						 const std::string &select,
						 const std::string &where,
						 std::function<void (int argc, char **argv,
											 char **azColName) noexcept>
						 callback)
		{
			std::string cmd;
			if (where.size ())
				cmd = "select " + select + " from " + tableName +
				" where (" + where + ");";
			else
				cmd = "select " + select + " from " + tableName + ";";
			Excute (cmd, std::move (callback));
		}

		~SQLConnector ()
		{
			sqlite3_close (db);
		}

	private:
		sqlite3 *db;
		static void _callback (int argc, char **argv, char **azColName)
		{ return; }
	};
}

#endif // !POKEMON_SQL_H