## Ubuntu Installation

```sh
sudo apt-get install libjsoncpp-dev
gcc -o santacpp Puzzle.cpp Santa2023.cpp -I. -I/usr/include/jsoncpp -L/usr/lib/x86_64-linux-gnu/ -ljsoncpp -fopenmp -O3 -lstdc++fs -lstdc++ -std=c++11 -lm
```

## Simulated Annealing Usage
```sh
./santacpp sa puzzles.csv puzzle_info.csv <submission_file> <puzzle_id> <initial_temperature> <cooling_rate> <minimum_temperature> <max_retries> <max_sequence_length>
```

Options:
- submission_file: initial submission file, for which puzzle_id will be replaced with the new solution
- puzzle_id: id of the puzzle to solve
- initial_temperature: initial temperature for the simulated annealing algorithm
- cooling_rate: cooling rate for the simulated annealing algorithm
- minimum_temperature: minimum temperature for the simulated annealing algorithm, when reached the algorithm backtracks to the best solution found and resets the temperature
- max_retries: maximum times the algorithm will backtrack to the same best solution, if reached the algorithm discards it and backtracks to the 2nd best solution
- max_sequence_length: maximum length of the sequence of moves to try for each iteration of the algorithm, the sequence length is randomly chosen between 1 and max_sequence_length and the moves are randomly chosen from the available moves

### Example

I believe the following parameters were able to find a feasible solution for puzzle 337 in 20 minutes on my Ryzen 9 5900x machine:
```sh
./santacpp sa .\puzzles.csv .\puzzle_info.csv .\submission.csv 337 1000 0.99 0.4 2 8
```

The solution will be long and need to be locally optimized.