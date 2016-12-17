# Pokemon Protocol

## Data

``` json
user = {"uid": uid, "online": isonline, "wonrate": wonrate,
        "badges": [badge], "pokemons": [pokemon]}
```

``` json
pokemon = {"pid": pid, "uid": uid, "name": name,
           "level": level, "exppoint": exppoint,
           "atk": atk, "def": def, "hp": hp, "timegap": timegap}
```

``` json
roomplayer = {"uid": uid, "ready": isready, "pokemon": pokemon}
```

``` json
gameplayer = {"uid": uid,
              "x": x, "y": y,
              "vx": vx, "vy": vy,
              "timegap": timegap,
              "curHp": curHp}
```

``` json
resultplayer = {"uid": uid,
                "won": iswon,
                "pokemon": pokemonlost}
```

## Accounting

#### register

``` json
{"request": "register", "param": {"uid": uid, "pwd": pwd}}

{"success": true, "response": msg}
{"success": false, "response": msg}
```

#### login

``` json
{"request": "login", "param": {"uid": uid, "pwd": pwd}}

{"success": true, "response": {"sid": sid, "user": thisuser}
{"success": false, "response": msg}
```

#### logout

``` json
{"request": "logout", "param": {"sid": sid}}

{"success": true, "response": msg}
{"success": false, "response": msg}
```

## Query

#### pokemons

``` json
{"request": "pokemons", "param": {"sid": sid}}

{"success": true, "response": [pokemon]}
{"success": false, "response": msg}
```

#### users

``` json
{"request": "users", "param": {"sid": sid}}

{"success": true, "response": [user]}
{"success": false, "response": msg}
```

## Room

#### rooms

``` json
{"request": "rooms", "param": {"sid": sid}}

{"success": true, "response": [rid]}
{"success": false, "response": msg}
```

#### roomenter

``` json
{"request": "roomenter",
 "param": {"sid": sid, "rid": rid, "pid": pid}}

{"success": true, "response": msg}
{"success": false, "response": msg}
```

#### roomleave

``` json
{"request": "roomleave", "param": {"sid": sid}}

{"success": true, "response": msg}
{"success": false, "response": msg}
```

#### roomready

``` json
{"request": "roomready",
 "param": {"sid": sid, "ready": isready}}

{"success": true, "response": [roomplayer]}
{"success": false, "response": msg}
```

## Gaming

#### gamesync

``` json
{ "request": "gamesync",
  "param": {
    "action": {
      "timestamp": timestamp,
      "movex": movex, "movey": movey,
      "atkx": atkx, "atky": atky, "def": isdef}
    }
}

{
  "success": true,
  "response": {
    "over": isover,
    "players": [gameplayer]   // not over
    "players": [resultplayer] // over
  }
}
{"success": false, "response": msg}
```
