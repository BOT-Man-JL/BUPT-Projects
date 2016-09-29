#ifndef POKEMON_SQL_H
#define POKEMON_SQL_H

#include <string>
#include <functional>
#include <thread>
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

		// Don't throw in callback
		void Excute (const std::string &cmd,
					 std::function<int (int argc, char **argv,
										char **azColName) noexcept> callback)
		{
			const size_t MAX_TRIAL = 16;
			char *zErrMsg = 0;
			int rc;

			auto callbackWrapper = [] (void *fn, int argc, char **argv, char **azColName)
			{
				return static_cast<std::function<int (int argc, char **argv,
													  char **azColName) noexcept> *> (fn)
					->operator()(argc, argv, azColName);
			};

			for (size_t iTry = 0; iTry < MAX_TRIAL; iTry++)
			{
				rc = sqlite3_exec (db, cmd.c_str (), callbackWrapper, &callback, &zErrMsg);
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
			auto cmd = "create table " + tableName + " (" + params + ");";
			try
			{
				Excute (cmd, _callback);
			}
			catch (std::exception &e)
			{
				throw std::move (e);
			}
		}

		void DropTable (const std::string &tableName)
		{
			auto cmd = "drop table " + tableName + ";";
			try
			{
				Excute (cmd, _callback);
			}
			catch (std::exception &e)
			{
				throw std::move (e);
			}
		}

		void InsertValue (const std::string &tableName,
						  const std::string &value)
		{
			auto cmd = "insert into " + tableName + " values (" + value + ");";
			try
			{
				Excute (cmd, _callback);
			}
			catch (std::exception &e)
			{
				throw std::move (e);
			}
		}

		void DeleteValue (const std::string &tableName,
						  const std::string &where)
		{
			std::string cmd;
			if (where.size ())
				cmd = "delete from " + tableName + " where " + where + ";";
			else
				cmd = "delete * from " + tableName + ";";
			try
			{
				Excute (cmd, _callback);
			}
			catch (std::exception &e)
			{
				throw std::move (e);
			}
		}

		void QueryValue (const std::string &tableName,
						 const std::string &select,
						 const std::string &where,
						 std::function<int (int argc, char **argv,
											char **azColName) noexcept> callback)
		{
			std::string cmd;
			if (where.size ())
				cmd = "select " + select + " from " + tableName + " where " + where + ";";
			else
				cmd = "select " + select + " from " + tableName + ";";
			try
			{
				Excute (cmd, std::move (callback));
			}
			catch (std::exception &e)
			{
				throw std::move (e);
			}
		}

		~SQLConnector ()
		{
			sqlite3_close (db);
		}

	private:
		sqlite3 *db;
		static int _callback (int argc, char **argv, char **azColName) { return 0; }
	};
}

#endif // !POKEMON_SQL_H