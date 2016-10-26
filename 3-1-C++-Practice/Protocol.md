# Pokemon Protocol

- **PokemonServer** is the Server who handle Request from Client;
- **PokemonClient** is the Client who send Request to Server;

## Session

Keep Alive by Default;

## Format

### Request Format

> Token \n
> Param 1 \n
> Param 2 \n
> ...
> \0

### Response Format

If Succeeded:

> 1 \n
> Param 1 \n
> Param 2 \n
> ...
> \0

If Failed:

> 0 \n
> Error Msg
> \0

## Tokens and Params

> Request Params
>
> Response Params

### Control

### Accounting

#### Login

> User ID \n
> User Password
>
> Session ID

#### Register

> User ID \n
> User Password
>
> Msg

#### Logout

> Session ID
>
> Msg

### User/Pokemon Info

#### UsersWonRate

> Session ID \n
> User ID
>
> Won Rate (double)

#### UsersBadges

> Session ID \n
> User ID
>
> Badge 1 \n
> Badge 2 \n
> ...

#### UsersAll

> Session ID \n
>
> User ID 1 \n
> User ID 2 \n
> ...

#### UsersOnline

> Session ID \n
>
> User ID 1 \n
> User ID 2 \n
> ...

#### UsersPokemons

> Session ID \n
> User ID
>
> Pokemon ID 1 \n
> Pokemon ID 2 \n
> ...

#### PokemonsAll

> Session ID \n
>
> Pokemon ID 1 \n
> Pokemon ID 2 \n
> ...

#### PokemonInfo

> Session ID \n
> Pokemon ID \n
>
> Name \n
> Level \n
> Exp Point \n
> Atk \n
> Def \n
> HP \n
> FullHP \n
> Time Gap \n