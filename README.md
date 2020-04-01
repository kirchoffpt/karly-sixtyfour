# Karly64 
Chess engine written in C++11

Version 0.8.0 (Windows only)

A 64-bit, mostly UCI compliant, AB minimax engine. Elo is approximately 2000 depending on the time control and various other factors. This engine is a hobby project and was (and is being) written completely from scratch with relatively little reference to other engines/wikis because it's more fun that way.

**Play it [here](https://lichess.org/@/karly64)!** Lichess account needed (free).

### Features

#### Move Generation
Features a novel legal-move generation algorithm that effectively generates moves for both sides on every ply. The method relies heavily on reusing information from previous positions making it non-memoryless and quite difficult to debug. But while *very* tedious to implement this method saves considerable time by only fully updating moves for pieces that were affected by the last move. And much more importantly it makes symmetric evaluations based on mobility or controlled squares extremely fast and trivial to implement.

The move generator is currently about half as fast as the *very* quick [qperft](https://home.hccnet.nl/h.g.muller/perft.c) by H.G. Muller (nearly 200 million moves per second) but the current implementation of Karly64's move generator still leaves a lot of obvious opportunities for optimizations. This generator is also pulling double duty for evaluation's sake.

It is possible this move generation scheme will end up being unimportant or even cumbersome in the scope of an entire chess engine but a general outline of the algorithm is laid out in [movegen.txt](./movegen.txt) (or see the current implementation in `chess_pos::generate_moves()` in [chess_pos.cpp](./src/chess_pos.cpp)) if anyone wants ideas from it.

#### Evaluation
Notably this engine does not and will not use any piece square tables (a very common method assigning predetermined scores for certain pieces on certain squares). Its general strategy is to try to maximize controlled squares and keep a safe king. At the moment king safety evaluations are based on attempting to minimize the number of ways an enemy piece can get into the king's general area.

Evaluation also uses flood fill algorithms for various king-related evaluations. For example even if the engine can't see a distant checkmate with two bishops it can usually do a *reasonably* good job of constraining the enemy king's move space until it can. The engine also likes to keep the enemy from controlling too many squares in the vicinity of its king. 

The evaluation in general works quite well for the time being (it used to perform ridiculous castles when it was naively using piece square tables). At the moment only changes to the search (making it better at looking at critical lines) are going to have any notable effect on performance.

That being said it will be interesting to later try and work a neural network in somewhere to assist in deciding between moves that score similarly or assist with move ordering.
#### Everything Else
Everything else in the engine is pretty standard stuff you can find on the [chess programming wiki](https://www.chessprogramming.org/Main_Page), but here are current and planned implementations. Endgame tablebases are not currently planned.
  - Current
    - Bitboards (64 bit board representations)
    - Fast legal move generation (as opposed to pseudo-legal)
    - Principal variation search (minimax variant) w/ iterative deepening
    - Quiescence/Capture search
    - Null move pruning (compile time option)
    - Transposition table w/ Zobrist hashing
    - Late Move Reductions (LMR)
    - UCI protocol (more than good enough for most GUI's)
  - Planned
    - More advanced quiescence search and draw detection
    - Chess960 support
    - Multithreading capabilities
    - More/improved forward pruning
    - Neural net assisted move ordering
    - Small optimizations to move generation
    - More fleshed out time management
    
   
   
 ##### Named after my friend that is absolutely [terrible at chess](https://lichess.org/@/heatner). He was honored.
