
//
// Pokemon Game - Shared Module
// BOT Man, 2016
//

#ifndef POKEMON_SHARED_H
#define POKEMON_SHARED_H

#include <string>

namespace PokemonGame
{
	using PokemonID = size_t;
	using PokemonName = std::string;

	using UserID = std::string;
	using UserPwd = std::string;
	using UserBadge = std::string;

	using SessionID = std::string;
	using RoomID = std::string;

	constexpr const char *BadSession = "You haven't Login";
}

#endif // !POKEMON_SHARED_H