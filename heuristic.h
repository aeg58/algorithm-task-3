#ifndef HEURISTIC_H
#define HEURISTIC_H

#include "common.h"

void createSequentialBaseline(const Problem *problem, Solution *solution);
void createNearestNeighborTwoOptSolution(const Problem *problem, Solution *solution);
void createILSTwoOptSolution(const Problem *problem, Solution *solution, double timeLimitSeconds);
double getKnownReferenceLength(const Problem *problem);
void printSolutionRoutes(const Problem *problem, const Solution *solution);

#endif
