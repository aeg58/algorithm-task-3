Task 3 - Vehicle Routing Problem / Heuristic Assignment - V3

This project solves the Vehicle Routing Problem (VRP) using modular C code.
The implementation focuses first on route correctness and then on solution quality.

============================================================
IMPORTANT: TWO OUTPUT MODES
============================================================

1) Plain checker mode - recommended for lecturer/checker testing
--------------------------------------------------------------
Use this mode if the lecturer wants a simple output format similar to the assignment example.
It prints only:
- first line: total distance
- following line(s): vehicle route(s)

Compile:
gcc -Wall -Wextra -std=c11 main.c input.c distance.c validation.c heuristic.c exact.c -o task3 -lm

Run plain mode:
./task3 --plain < input/in.txt

Save plain output:
./task3 --plain < input/in.txt > output/out_plain.txt

For the provided tests:
./task3 --plain < input/test_1car.txt > output/out_1car_plain.txt
./task3 --plain < input/test_2cars.txt > output/out_2cars_plain.txt

2) Detailed report mode - used for analysis and presentation
------------------------------------------------------------
This mode prints a readable comparison report with validation details, distances, execution times,
known reference gap values, and the best valid heuristic routes.
This is useful for explaining the solution to the lecturer.

Run detailed mode:
./task3 < input/in.txt

Save detailed output:
./task3 < input/in.txt > output/out.txt

For the provided tests:
./task3 < input/test_1car.txt > output/out_1car.txt
./task3 < input/test_2cars.txt > output/out_2cars.txt

============================================================
PROJECT STRUCTURE
============================================================

- main.c: program flow, output modes, comparison table
- common.h: shared constants and structs
- input.c / input.h: input reading
- distance.c / distance.h: distance matrix and route distance calculation
- validation.c / validation.h: independent route validation checks
- heuristic.c / heuristic.h: baseline, nearest neighbor, 2-opt, ILS
- exact.c / exact.h: exact backtracking for small one-vehicle instances
- input/: example and lecturer test files
- output/: detailed and plain generated outputs
- docs/: notes and optimal_solutions.pdf

============================================================
INPUT FORMAT
============================================================

clientCount vehicleCount maxClientsPerVehicle
client1_x client1_y
client2_x client2_y
...
clientN_x clientN_y
depot_x depot_y

In the code:
- depot is represented as 0
- clients are represented as 1..clientCount

============================================================
ALGORITHMS
============================================================

1. Naive Sequential Baseline
   Simple reference solution. It is intentionally basic and not expected to be good.

2. Nearest Neighbor + 2-opt
   Construction heuristic + local optimization.

3. ILS + 2-opt
   Time-limited Iterated Local Search.
   It uses semi-random construction, random perturbation, inter-route swap for multi-vehicle cases,
   and 2-opt improvement. The default time limit is 2 seconds.

4. Exact Backtracking
   Runs only for small one-vehicle instances.
   For large inputs such as 400 clients, it is intentionally skipped because exact TSP/VRP search
   grows factorially and is not practical.

============================================================
VALIDATION CHECKS
============================================================

Every generated solution is checked independently:
- every route starts at depot
- every route ends at depot
- every client is visited exactly once
- no client is visited twice
- maximum clients per vehicle is respected
- total distance is recalculated independently
- stored distance is compared against recalculated distance

============================================================
KNOWN REFERENCE VALUES
============================================================

The known reference values from docs/optimal_solutions.pdf are:
- 1 vehicle: 15300
- 2 vehicles: 15500

These values are used only for reporting the percentage gap.
They are not used by any algorithm to construct routes.
