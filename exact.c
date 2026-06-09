#include <stdio.h>
#include <string.h>
#include <time.h>
#include "exact.h"

/*
 * exact.c
 *
 * Bu dosyada küçük inputlar için exact backtracking solver bulunur.
 * 400 müşteri için exact çözüm pratik değildir; bu yüzden sadece küçük instance'larda çalıştırılır.
 */

static double bestDistance;
static int bestRoute[MAX_ROUTE_SIZE];
static int currentRoute[MAX_ROUTE_SIZE];
static int visited[MAX_CLIENTS + 1];

/*
 * Fonksiyon amacı:
 * TSP backtracking recursive araması yapar.
 *

 */
static void exactBacktrack(const Problem *problem, int level, int currentVertex, double currentDistance) {
    if (currentDistance >= bestDistance) {
        return;
    }

    if (level == problem->clientCount) {
        double totalDistance = currentDistance + problem->distanceMatrix[currentVertex][DEPOT];

        if (totalDistance < bestDistance) {
            bestDistance = totalDistance;

            for (int i = 0; i <= problem->clientCount; i++) {
                bestRoute[i] = currentRoute[i];
            }

            bestRoute[problem->clientCount + 1] = DEPOT;
        }

        return;
    }

    for (int client = 1; client <= problem->clientCount; client++) {
        if (!visited[client]) {
            visited[client] = 1;
            currentRoute[level + 1] = client;

            exactBacktrack(
                problem,
                level + 1,
                client,
                currentDistance + problem->distanceMatrix[currentVertex][client]
            );

            visited[client] = 0;
        }
    }
}

/*
 * Fonksiyon amacı:
 * Küçük input için exact backtracking solver çalıştırır.
 * Büyük inputlarda bilinçli olarak SKIPPED sonucu üretir.
 *

 */
void runExactBacktrackingForSmallInstance(const Problem *problem, ExactResult *result) {
    memset(result, 0, sizeof(ExactResult));
    clock_t start = clock();

    if (problem->vehicleCount != 1) {
        result->skipped = 1;
        snprintf(result->message, sizeof(result->message),
                 "SKIPPED: exact backtracking is implemented only for one-vehicle small instances.");
        result->executionTimeSeconds = (double)(clock() - start) / CLOCKS_PER_SEC;
        return;
    }

    if (problem->clientCount > EXACT_MAX_CLIENTS) {
        result->skipped = 1;
        snprintf(result->message, sizeof(result->message),
                 "SKIPPED: instance too large for exact factorial search (clientCount > %d).", EXACT_MAX_CLIENTS);
        result->executionTimeSeconds = (double)(clock() - start) / CLOCKS_PER_SEC;
        return;
    }

    if (problem->maxClientsPerVehicle < problem->clientCount) {
        result->skipped = 1;
        snprintf(result->message, sizeof(result->message),
                 "SKIPPED: one vehicle cannot serve all clients because of capacity limit.");
        result->executionTimeSeconds = (double)(clock() - start) / CLOCKS_PER_SEC;
        return;
    }

    bestDistance = INF;
    memset(visited, 0, sizeof(visited));
    memset(bestRoute, 0, sizeof(bestRoute));
    memset(currentRoute, 0, sizeof(currentRoute));

    currentRoute[0] = DEPOT;
    exactBacktrack(problem, 0, DEPOT, 0.0);

    result->available = 1;
    result->bestDistance = bestDistance;
    result->routeSize = problem->clientCount + 2;

    for (int i = 0; i < result->routeSize; i++) {
        result->route[i] = bestRoute[i];
    }

    snprintf(result->message, sizeof(result->message), "Exact backtracking completed for small instance.");
    result->executionTimeSeconds = (double)(clock() - start) / CLOCKS_PER_SEC;
}

/*
 * Fonksiyon amacı:
 * Exact solver sonucunu yazdırır.
 *

 */
void printExactResult(const ExactResult *result) {
    printf("\nExact Backtracking Result:\n");
    printf("Status: %s\n", result->available ? "AVAILABLE" : "SKIPPED");
    printf("Message: %s\n", result->message);
    printf("Execution time: %.6f seconds\n", result->executionTimeSeconds);

    if (result->available) {
        printf("Exact distance: %.6f\n", result->bestDistance);
        printf("Exact route:");
        for (int i = 0; i < result->routeSize; i++) {
            printf(" %d", result->route[i]);
        }
        printf("\n");
    }
}
