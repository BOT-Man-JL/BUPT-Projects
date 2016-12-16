# Pokemon Protocol

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

{"success": true, "response": sid}
{"success": false, "response": msg}
```

#### logout

``` json
{"request": "logout", "param": {"sid": sid}}

{"success": true, "response": msg}
{"success": false, "response": msg}
```

## Query

``` json
user = {"uid": uid, "wonrate": wonrate,
        "badges": [badge], "pokemons": [pokemon]}
```

``` json
pokemon = {"pid": pid, "uid": uid, "name": name,
           "level": level, "exppoint": exppoint,
           "atk": atk, "def": def, "hp": hp, "timegap": timegap}
```

#### pokemonall

``` json
{"request": "pokemonall", "param": {"sid": sid}}

{"success": true, "response": [pokemon]}
{"success": false, "response": msg}
```

#### userall

``` json
{"request": "userall", "param": {"sid": sid}}

{"success": true, "response": [user]}
{"success": false, "response": msg}
```

#### useronline

``` json
{"request": "useronline", "param": {"sid": sid}}

{"success": true, "response": [user]}
{"success": false, "response": msg}
```

## Room

#### roomall

``` json
{"request": "roomall", "param": {"sid": sid}}

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
{"request": "roomready", "param": {"sid": sid}}

{"success": true, "response": msg}
{"success": false, "response": msg}
```

#### roomstate

``` json
roomplayer = {"uid": uid, "isready": isready, "pokemon": pokemon}
```

``` json
{"request": "roomstate", "param": {"sid": sid}}

{"success": true, "response": [roomplayer]}
{"success": false, "response": msg}
```

## Gaming

``` json
player = {"uid": uid, "x": x, "y": y, "vx": vx, "vy": vy,
          "curHp": curHp, "pokemon": pokemon}
```

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

{"success": true, "response": [player]}
{"success": false, "response": msg}
```

#### gameresult

``` json
playerresult = {"uid": uid, "win": iswin, "pokemon": pokemon}
```

``` json
{"request": "gameresult", "param": {"sid": sid}}

{"success": true, "response": [playerresult]}
{"success": false, "response": msg}
```