#ifndef VALIDATION_H
#define VALIDATION_H

#include "common.h"

int routesStartAndEndAtDepot(const Problem *problem, const Solution *solution);
int allClientsVisitedExactlyOnce(const Problem *problem, const Solution *solution);
int routesRespectMaxClientsPerVehicle(const Problem *problem, const Solution *solution);
void validateSolution(const Problem *problem, Solution *solution);
void printValidationDetails(const Solution *solution);

#endif
