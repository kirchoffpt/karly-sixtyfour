# Karly64 
Chess engine written in C++11

Version 0.9.x

A 64-bit, mostly [UCI](http://wbec-ridderkerk.nl/html/UCIProtocol.html) compliant, AB minimax engine. Elo is approximately 2000 depending on the time control and various other factors. This engine is a hobby project and is being written completely from scratch.

**Play it [here](https://lichess.org/@/karly64)!** Lichess account needed (free).

I left the old readme if anyone actually wants to read that stuff. Most things in the engine are pretty standard however a lot of implementation was done with little reference. The move generation algorithm is pretty clever (incrementally updates list of moves as most legal moves stay the same after a single piece is moved) and very fast but its overall performance criticality is questionable; It will also make your eyes bleed. 

The project started as more low-level C style with C++ classes thrown in but I'm currently in the process of refactoring, adding tests, and just generally messing with things.

## BUILD
Just cmake and build. The cmake is setup to download googletest automatically. Should work for both g++ and msvc.
If on linux just run the following in the root folder:
```
cmake .
make
./karly64
```

I can share the dockerfile for full deployment of a lichess bot if anyone wants it.

Reminder that this is a command line engine and does not come with its own gui. Arena chess works quite well.

## TEST
After building just use ctest or manually run testall for a more detailed breakdown. May be slightly different depending on the build type.

There is also a perft executable (counts moves of different positions to certain depths) that will probably be deprecated.

