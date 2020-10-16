# Karly64 
Chess engine written in C++11

Version 0.9.x

A 64-bit, mostly [UCI](http://wbec-ridderkerk.nl/html/UCIProtocol.html) compliant, AB minimax engine. Elo is approximately 2000 depending on the time control and various other factors. This engine is a hobby project and is being written completely from scratch.

**Play it [here](https://lichess.org/@/karly64)!** Lichess account needed (free).

I left the old readme if anyone actually wants to read that stuff. Most things in the engine are pretty standard. The move generation algorithm is pretty clever (incrementally updates list of moves as most legal moves stay the same after a single piece is moved) and very fast but it's overall performance criticality is questionable; It will also make your eyes bleed. 

I'm currently in the process of refactoring, adding tests, and just generally messing with things.

## BUILD
Just cmake the src folder and then build (try `cmake -S ./src -B ./bin`). Should work for both g++ and msvc. 

I can share the docker project for full deployment of a lichess bot if anyone wants it.

Reminder that this is a command line engine and does not come with its own gui.
