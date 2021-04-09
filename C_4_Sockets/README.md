# Server
## Goncharov Mark

The MIPT project. The main target is to create stable TCP/UDP server, 
to control programming style, to add many useful properties, 
to pass different tests.

### Description
1. Install
2. Usage
3. How it works
4. Version

Install
=======
1. clone this repository
2. ```mkdir build```
3. ```make```

Usage
=====
1. ```make``` - compile program files
2. ```make server``` - creating daemon server
3. ```make client``` - creating client
4. ```make serverTCP``` and ```make clientTCP``` the same
5. Write ```exit``` in client in order to exit ##### client
6. Write ```CLOSE_SERVER``` in client in order to close ##### server
7. In order to fast closing UDP server can be used ```make close```
8. Use ```bash``` in client in order to use client like ordinary terminal


How it works
============
1. Client is sending request to server in order to get ID (UDP)
2. Server is searching free ID in ID_base
3. Returns ID to client and creating slave that will work on this client
4. Server now just check ID of new messages and translating them to slaves accordingly
5. Slaves just translate client's requst and give them necessary information
![Scheme of work](images/1.png)

Version
========
Requirements
------------
1. TCP/UDP switch for server and client
2. Remote shell using ptmx
3. Server must work in daemon mode
4. Server must be broadcasted by client
5. Clients must be isolated to each other
6. Erros must be handling and processed properly (not killed process)
7. Everything must be logging (info/warning/error)
8. Should be control over loosing UDP packets
9. We have to use buildsystem like make/cmake and so on
10. Should be linked using own shared libraries
11. In order to save information should be used cryptography
12. Program should pass tests
13. Every program should properly handle signals like SIGKILL and so on

Current version
---------------
| Task | Done | Preparing                         |
| :---:|:----:|:---------------------------------:|
| 1    | 80%  | Common usage functions            |
| 2    | 100% | Done                              |
| 3    | 100% | Done                              |
| 4    | 100% | Done                              |
| 5    | 100% | Done                              |
| 6    | 90%  | Returning values can be optimized |
| 7    | 100% | Done                              |
| 8    | 0%   | Not started                       |
| 9    | 100% | Done                              |
| 10   | 20%  | Linking -c for libs               |
| 11   | 0%   | Soon                              |
| 12   | 0%   | Tests weren't prepared            |
| 13   | 50%  | Client is reaining                |

Logging versions
----------------
08.04
> Start README. Current targets:
> 1. Optimize return value handling in functions
> 2. Logging into /var/log
> 3. Killing server more accurate
> 4. Rewrite CheckNewClient without switch and so on
> 5. One function-name style must be used
> 6. Rename functions: GetID, AddID and so on...
> 7. Close file-descr before exec
> 8. Packet->data_ should be union for convient usage
> 9. fgets can not be used in order to save from dup2
> 10. Signal handling (can be used tcsetattr/cfmakeraw)
> 11. Common usage for TCP and UDP

09.04
> Done: 1, 4