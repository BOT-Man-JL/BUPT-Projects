# Naive Async Crawler

> by BOT Man & ZhangHan, 2018

## Features

- use [libevent](https://libevent.org) to process async IO
- use [libwww](https://dev.w3.org/libwww/Library/src/HTParse.html) to parse and canonicalize URL(URI)
- use [bloom filter](https://en.wikipedia.org/wiki/Bloom_filter) to implement url hash set
- use [deterministic finite automaton (DFA)](https://en.wikipedia.org/wiki/Deterministic_finite_automaton) to parse `<a>` tag urls inside html
- use [TAILQ](https://linux.die.net/man/3/queue) to implement pending request queue

## Requirements

``` bash
# libevent
sudo apt-get install libevent-dev

# Visual Studio Linux Development
sudo apt-get install openssh-server g++ gdb gdbserver
sudo service ssh start
```

## Compile

``` bash
clang++ crawler/*.c crawler/*.cpp crawler/third_party/*.c -Wall -levent -o crawler.out
```

## Test Website

``` bash
# nginx
sudo apt-get install nginx
sudo nginx
sudo cp -r www/* /var/www/html/

./crawler.out localhost/
```

## Internals

### Trans-State Diagram

```
                     DoInit   DoConn   DoSend   DoRecv
                        \        \        \        \
 (CreateState) --> Init --> Conn --> Send --> Recv --> Succ --:
                    :        :        :        :              :--> (FreeState)
                    :--------:--------:--------:-----> Fail --:
```

### Trans-State Table

Old State | New State | Old Event | New Event | Old Buffer | New Buffer
---|---|---|---|---|---
Init | Conn | NULL | EV_WRITE + DoConn | NULL | NULL
Conn | Send | EV_WRITE + DoConn | EV_WRITE + DoSend | NULL | Send Buffer
Send | Recv | EV_WRITE + DoSend | EV_READ + DoRecv | Send Buffer | NULL
Recv | Succ | EV_READ + DoRecv | NULL | Recv Buffer | NULL
? | Fail | ? | NULL | ? | NULL

### How to extract HTParse.h

``` js
// 1. visit https://dev.w3.org/libwww/Library/src/HTParse.html
// 2. extract header in browser console
let ret = '';
for (const pre of document.getElementsByTagName('pre'))
  ret += '\n' + pre.innerHTML;
```

## TODO

- [ ] Parse [PORT](https://en.wikipedia.org/wiki/Uniform_Resource_Identifier#Syntax) from URL authority
- [ ] Handle [URL redirection](https://en.wikipedia.org/wiki/URL_redirection)
- [ ] Support [chunked transfer encoding](https://en.wikipedia.org/wiki/Chunked_transfer_encoding#Encoded_data)
