
#include <string>
#include <iostream>
#include <vector>

#include "Socket.h"
#include "SQL.h"

int main (int argc, char *argv[])
{
	std::cout << "Pokemon Server" << std::endl;

	PokemonGame_Impl::Server (5768, [] (const std::string &request,
										std::string &response)
	{
		std::cout << request << std::endl;
		response = "Echo: " + request;

		PokemonGame_Impl::SQLConnector connector ("Pokemon.db");

		std::vector<std::string> cmds
		{
			"create table tbl1(one varchar(10), two smallint);",
			"insert into tbl1 values ('hello!',10);",
			"insert into tbl1 values ('goodbye', 20);",
			"select * from tbl1;"
		};

		auto callback = [] (void *, int argc, char **argv, char **azColName)
		{
			for (size_t i = 0; i < argc; i++)
				printf ("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
			printf ("\n");
			return 0;
		};

		for (const auto &cmd : cmds)
		{
			try
			{
				connector.Excute (cmd, callback);
			}
			catch (const std::exception& e)
			{
				std::cerr << e.what () << std::endl;
				//break;
			}
		}
	});

	getchar ();
	return 0;
}