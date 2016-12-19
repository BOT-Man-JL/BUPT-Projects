# Pokemon Protocol

## Object

#### sesseion error

``` json
{"success": false, "response": "You haven't Login"}
```

#### user

``` json
{
  "uid": uid, "online": isonline, "wonrate": wonrate,
  "badges": [badge], "pokemons": [pokemon]
}
```

#### pokemon

``` json
{
  "pid": pid, "uid": uid, "name": name,
  "level": level, "exppoint": exppoint,
  "atk": atk, "def": def, "hp": hp, "timegap": timegap
}
```

#### room

``` json
{
  "rid": rid,
  "pending": not_over_and_not_started
}
```

#### roomplayer

``` json
{
  "uid": uid, "ready": isready,
  "width": width, "height": height,
  "pokemon": pokemon
}
```

#### gameplayer

``` json
{
  "uid": uid,
  "x": x, "y": y,
  "vx": vx, "vy": vy,
  "timegap": timegap,
  "hp": curHp
}
```

#### gamedamage

``` json
{
  "did": did,
  "x": x, "y": y,
  "vx": vx, "vy": vy
}
```

#### resultplayer

``` json
{
  "uid": uid,
  "won": iswon
}
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

{"success": true, "response": {"width": width, "height": height}}
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
    "sid": sid,
    "movex": movex, "movey": movey,
    "atkx": atkx, "atky": atky,
    "def": isdef
  }
}

{
  "success": true,
  "response": {
    "over": isover,
    "gameplayers": [gameplayer],     // not over
    "gamedamages": [gamedamage],     // not over
    "resultplayers": [resultplayer]  // over
  }
}
{"success": false, "response": msg}
```
