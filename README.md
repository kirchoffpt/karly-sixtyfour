# Karly64 
Chess engine written in C++11

Version 0.1.1 (Windows only)

A 64-bit, mostly UCI compliant, AB minimax engine with a simple quiescence search. It is quite strong at the moment considering how basic it is. Elo is approximately 1500-2000 depending on the time control and if it's playing a human or another engine. This engine is a hobby project and was (and is being) written completely from scratch with relatively little reference to other engines/wikis because it's more fun that way.

Play it [here](https://lichess.org/@/karly64)! Lichess account needed (free).

### Features

#### Move Generation
Features a novel legal-move generation algorithm that effectively generates moves for both sides on every ply. The method relies heavily on reusing information from previous positions making it non-memoryless and quite difficult to debug. But while *very* tedious to implement this method saves considerable time by only fully updating moves for pieces that were affected by the last move. And more importantly it makes symmetric evaluations based on mobility or controlled squares extremely fast and trivial to implement.

The move generator is currently about half as fast as the *very* quick [qperft](https://home.hccnet.nl/h.g.muller/perft.c) by H.G. Muller (nearly 200 million moves per second) but the current implementation of Karly64's move generator still leaves a lot of obvious opportunities for optimizations. This generator is also pulling double duty for evaluation's sake.

It is possible this move generation scheme will end up being unimportant or even cumbersome in the scope of an entire chess engine but a general outline of the algorithm is laid out in [movegen.txt](./movegen.txt) if anyone wants ideas from it.

#### Everything Else
Everything else in the engine is pretty standard stuff you can find on the [chess programming wiki](https://www.chessprogramming.org/Main_Page), but here are current and planned implementations.
  - Current
    - Bitboards (64 bit board representations)
    - Fast legal move generation (as opposed to pseudo-legal)
    - Principal variation search (minimax variant) w/ iterative deepening
    - Quiescence/Capture search
    - Weak UCI implementation (good enough for most GUIs)
  - Planned
    - Transposition table
    - More advanced quiescence search and draw detection
    - Multithreading capabilities
    - More complete adherence to UCI protocol
    - Many more optimizations in general
    
   
   
 ##### Named after my [friend](https://lichess.org/@/heatner) that is absolutely terrible at chess. He was honored.
