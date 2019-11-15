# KARLY64 
Chess engine written in C++11

Version 0.1.1 (Windows only)

A 64-bit, mostly UCI compliant, AB minimax engine with a simple quiescence search. Elo is approximately 1500-2000 depending on the time control and if it's playing a human or another engine. This engine was written completely from scratch and is meant to be a hobby project. The goal is to eventually get above 2000 elo on the [CCRL rating list](https://www.computerchess.org.uk/ccrl/4040/). Karly64 is not currently on the rating list at it still missing some necessary elements to make it competitive. Karly64 currently generates 50 million to 100 million moves per second and evaluates 2 million to 6 million positions per second but usually does not search past depth 6 (3 moves) in the midgame.

Play it [here](https://lichess.org/@/karly64)! Lichess account needed (free).

### Features

#### Move Generation
Features a novel legal-move generation algorithm that keeps track of moves for both sides instead of only the side to move. Doing so actually allows for *faster* (compared to the previous side-to-move only implementation; see `chess_pos::generate_moves()` and `chess_pos::generate_moves_deprecated()`) move generation as the moves for many pieces need not be regenerated. Much more importantly, as most of time in a search is spent on evaluation, the wealth of information this method provides allows for fast and easy mobility and square-control based evaluations at the leaf nodes.

Note that this move generation method is more experimental and not strictly better. Although unlikely, it may prove to be cumbersome in the long run due to the lack of an unmake move function. However, it does seem to provide a lot more opportunities for optimizations.

#### Everything Else
Everything else in the engine is pretty standard stuff you can find on the chess programming wiki, but here are current and planned implementations.
  - Current
    - Bitboards (64 bit board representations)
    - Legal move generation (as opposed to pseudo-legal)
    - Alpha beta minimax search w/ iterative deepening
    - Quiescence/Capture search
    - Weak UCI implementation (good enough for most GUIs)
  - Planned
    - Transposition table
    - More advanced quiescence search and draw detection
    - Multithreading capabilities
    - More complete adherence to UCI protocol
    - Many more optimizations in general 
    
   
   
 ##### Named after my friend that is absolutely terrible at chess. He was honored.
