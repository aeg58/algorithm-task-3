#ifndef DISTANCE_H
#define DISTANCE_H

#include "common.h"

double calculateEuclideanDistance(double x1, double y1, double x2, double y2);
void buildDistanceMatrix(Problem *problem);
double calculateRouteDistance(const Problem *problem, const int route[], int routeSize);
double calculateTotalDistance(const Problem *problem, const Solution *solution);

#endif
