# Pokemon Protocol

- **PokemonServer** is the Server who handle Request from Client;
- **PokemonClient** is the Client who send Request to Server;

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

## Accounting

### Register

> User ID \n
> User Password
>
> Msg

### Login

> User ID \n
> User Password
>
> Session ID

### Logout

> Session ID
>
> Msg

## User Info

### UsersWonRate

> Session ID \n
> User ID
>
> Won Rate (double)

### UsersBadges

> Session ID \n
> User ID
>
> Badge 1 \n
> Badge 2 \n
> ...

### UsersAll

> Session ID \n
>
> User ID 1 \n
> User ID 2 \n
> ...

### UsersOnline

> Session ID \n
>
> User ID 1 \n
> User ID 2 \n
> ...

## Pokemon Info

### PokemonInfo

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

### UsersPokemons

> Session ID \n
> User ID
>
> Pokemon ID 1 \n
> Pokemon ID 2 \n
> ...

### PokemonsAll

> Session ID \n
>
> Pokemon ID 1 \n
> Pokemon ID 2 \n
> ...

## Room

### RoomQuery

> Session ID \n
>
> Room ID 1 \n
> Room ID 2 \n
> ...

### RoomEnter

> Session ID \n
> Room ID \n
> Pokemon ID \n
>
> Msg

### RoomLeave

> Session ID \n
>
> Msg

### RoomReady

> Session ID \n
>
> Msg

### RoomState

> Session ID \n
>
> Timestamp
> User ID \n
> Is Ready \n
> Init X \n
> Init Y \n
> Pokemon ID \n
> ...

## Playing

### Lockstep

> Session ID \n
> Action
>
> Timestamp \n
> Action 1 \n
> Action 2 \n
> ...

#### Action

> Action Type \t
> x (Move) \t
> y (Move) \t
> User ID \t
> Timestamp

#### Action Type

- None
- Move
  - x
  - y
- Attack
- Defend