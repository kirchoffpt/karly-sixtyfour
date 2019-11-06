# KARLY64 
Chess engine written in C++11

Version 0.1.0 (Windows only)

A 64-bit, mostly-UCI-compliant, AB minimax engine with a very simple quiescence search. Elo is approximately 1500-2000 depending on the time control and if it's playing a human or another engine. This engine was written completely from scratch. And although a rather useless metric, the engine evaluates, on average, about 3 million positions per second.

Play it [here](https://lichess.org/@/karly64)! Lichess account needed (free).

### Features

#### Move Generation
Features a novel legal-move generation algorithm (it is not pretty, see generate_moves() in chess_pos.cpp) that keeps track of moves for both sides instead of only the side to move. Doing so actually allows for *faster* (compared to the last implementation) move generation as the moves for many pieces need not be regenerated. Much more importantly, the wealth of information this method provides allows for fast and simple mobility based evaluations at the leaf nodes. 

Note that this move generation method is more experimental and not strictly better. Although unlikely, it may prove to be worse in the long run due to the lack of an unmake move function. However, it does seem to provide a lot more opportunities for optimizations.

#### Everything Else
Everything else in the engine is pretty standard stuff you can find on the chess programming wiki, but here are current and planned implementations.
  - Current
    - Bitboards (64 bit board representations)
    - Legal move generation (as opposed to pseduo-legal)
    - Alpha beta minimax search
    - Quiescence/Capture search
    - Weak UCI implementation (good enough for most GUIs)
  - Planned
    - Transposition table with zobrist keys
    - More advanced quiescence search and draw detection
    - Multithreading capabilites
    - Many more optimizations in general 
    
   
   
 ##### Named after my friend whom is terrible at chess. He was honored.
