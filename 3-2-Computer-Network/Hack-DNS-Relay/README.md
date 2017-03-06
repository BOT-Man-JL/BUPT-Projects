# Hacking DNS Relay

*DNS Relay Project* is an assignment of BUPT *Computer Network* Course.

> [Original Binary Source](dnsrelay.zip) (*exe*) archived in *zip*.
>
> [Hacked Code](dnsrelay.c) written in *C* language.
>
> [Sample Input](dnsrelay.txt) (hosts table) in *txt*.

## Internals Overview

### Platform

The original code is writtern on *Windows*, and this version is ported to *UNIX-like* systems.

On `Windows`:

- `s_addr` is defined as `S_un.S_addr`

On *UNIX-like* systems:

- `InitWSA` is not implemented
- `WSAGetLastError` is replaced as `errno`
- `BYTE`, `WORD` and `DWORD` are defined as `uint8_t`, `uint16_t`, `uint32_t`

### Design Details

#### Notations

- `BYTE`, `WORD` and `DWORD` are 8-bit, 16-bit and 32-bit `unsigned` respectively
- `pb*`, `pw*` and `pd*` are *pointers* to `BYTE`, `WORD` and `DWORD` respectively
- `sz*` is the *pointer* to C-style string

#### Global Variables

- `debugLevel` is used to control debug information output
- `curId` is ID used converting before forwarding to *name server*
- `sock` is the *socket* shared among functions
- `saNameServer` is the `sockaddr_in` of *name server*
- `pbTable` points to the *hosts table*

#### `main`

- Setup config and other global variables
- Initialize `sock` and Bind to *port 53*
- Call `LoadTable`
- Call `Run` in a dead loop

#### `LoadTable`

- Load *hosts table* from file to memory of `pbTable`, if the file presents
- Table format is `domain-name-string \0 dword-ip`
  - `domain-name` is converted to *lower case*
  - `dword-ip` is converted from IP string
- Memory of `pbTable` is allocated precisely

#### `Run`

- Call `recvfrom` and Fill the buffer
- Handle receiving error case (`length <= 0`)
- Handle response message from *name server* (`QR` == 1)
  - Restore original ID and socket address from `records`
  - Send this packet to *client*
- Handle query message to forward to *name server*
  - In following cases:
    - Not a simple Query (`QDCOUNT` != 1)
    - Non-standard Query (`OPCODE` != 0) || has-Authoritative-Answer (`AA` == 1)
    - Anwser not found in `pbTable` (described below)
  - Save the packet ID and socket address to `records`, and Replace the ID to `curId`
  - Send this packet to *name server*
- (Fall through) If not above cases, then
  - Retrieve *domain name*, *type* and *class* from query message
  - Find the *domain name* in `pbTable` if it's IPv4 message when table is not empty
    - Not Found => Forward this packet to *name server*
    - Found => Do as following
- Handle query message found in *hosts table*
  - Call `SetAnswer` with found entry
  - Send to *client* directly

#### `PrintTraffic`

- Print ip addr, port and buffer length
- If `mode` is NOT 2
  - Print buffer raw view and packet content

#### `SetAnswer`

- Set `QR` (Response) & `AA` (Authoritative answer) & `RA` (Recursion Available)
- If the anwser is NOT `0.0.0.0`
  - Set `ANCOUNT` to 1
  - Set name, type, class, ttl, rdlength and rddata
- Else
  - Set error response (`RCODE` = 3)
- Return the new size of buffer

## Authors and Credits

Hacked by [*BOT-Man-JL*](https://github.com/BOT-Man-JL)

Original Binary Code by [*gaozc@bupt.edu.cn*](mailto:gaozc@bupt.edu.cn)

Special thanks: [*thiefuniverse*](https://github.com/thiefuniverse) - helping debug on Linux