
#include <string>
#include <iostream>
#include <vector>

#include "Socket.h"
#include "SQL.h"

int main (int argc, char *argv[])
{
	// Init
	{
		PokemonGame_Impl::SQLConnector connector ("Pokemon.db");
		try
		{
			connector.CreateTable ("UserTbl",
								   "uid varchar(16) primary key, pwd varchar(16)");
			connector.InsertValue ("UserTbl", "'Admin', 'admin'");
			std::cout << "Created UserTbl\n";
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what () << std::endl;
		}
		try
		{
			connector.CreateTable ("PokemonTbl",
								   "id varchar(16) primary key, name varchar(16), master varchar(16)");
			std::cout << "Created PokemonTbl\n";
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what () << std::endl;
		}
		// Todo: Seed Data
	}

	std::cout << "Pokemon Server" << std::endl;

	PokemonGame_Impl::Server (5768, [] (const std::string &request,
										std::string &response)
	{
		std::cout << "I got '" << request << "'\n";
		response = "Echo: " + request;
		std::cout << "I said '" << response << "'\n";
		std::cout << std::endl;

		auto QueryCallback = [] (void *, int argc, char **argv, char **azColName)
		{
			for (size_t i = 0; i < argc; i++)
				std::cout << azColName[i] << " = "
				<< (argv[i] ? argv[i] : "NULL") << std::endl;
			std::cout << std::endl;
			return 0;
		};

		PokemonGame_Impl::SQLConnector connector ("Pokemon.db");
		try
		{
			connector.InsertValue ("UserTbl", "'John', 'JohnLee'");
			connector.QueryValue ("UserTbl", "*", "", QueryCallback);
			connector.DeleteValue ("UserTbl", "uid = 'John'");
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what () << std::endl;
		}
	});

	getchar ();
	return 0;
}