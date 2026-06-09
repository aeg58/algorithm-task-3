/*
 * Task 3 - Vehicle Routing Problem / Heuristic Assignment
 *
 * Modüler proje yapısı:
 * - input.c / input.h: input okuma
 * - distance.c / distance.h: mesafe matrisi ve mesafe hesaplama
 * - validation.c / validation.h: rota doğruluğu kontrolleri
 * - heuristic.c / heuristic.h: heuristic ve local search algoritmaları
 * - exact.c / exact.h: küçük inputlar için exact backtracking
 *


 */

#include <stdio.h>
#include <string.h>
#include "common.h"
#include "input.h"
#include "distance.h"
#include "validation.h"
#include "heuristic.h"
#include "exact.h"

/*
 * Fonksiyon amacı:
 * Bilinen referans değere göre yüzde gap hesaplar.
 *

 * This helper calculates percentage gap using the known reference value.
 */
static double calculateGapPercent(double distance, double knownReference) {
    if (knownReference <= 0.0) {
        return 0.0;
    }

    return ((distance - knownReference) / knownReference) * 100.0;
}

/*
 * Fonksiyon amacı:
 * Algoritma sonuçlarını tablo halinde karşılaştırır.
 *

 */
static void printComparisonTable(const Problem *problem, const Solution solutions[], int solutionCount, const ExactResult *exactResult) {
    double knownReference = getKnownReferenceLength(problem);

    printf("\n================ COMPARISON TABLE ================\n");
    printf("%-30s %-8s %-15s %-15s %-12s", "Algorithm", "Valid", "Distance", "Time(s)", "Dist.OK");

    if (knownReference > 0.0) {
        printf(" %-12s", "Gap(%)");
    }

    printf("\n");

    for (int i = 0; i < solutionCount; i++) {
        printf("%-30s %-8s %-15.6f %-15.6f %-12s",
            solutions[i].algorithmName,
            solutions[i].valid ? "YES" : "NO",
            solutions[i].totalDistance,
            solutions[i].executionTimeSeconds,
            solutions[i].distanceCheck ? "OK" : "MISMATCH"
        );

        if (knownReference > 0.0) {
            printf(" %-12.2f", calculateGapPercent(solutions[i].totalDistance, knownReference));
        }

        printf("\n");
    }

    if (exactResult->available) {
        printf("%-30s %-8s %-15.6f %-15.6f %-12s",
            "Exact Backtracking",
            "YES",
            exactResult->bestDistance,
            exactResult->executionTimeSeconds,
            "OK"
        );

        if (knownReference > 0.0) {
            printf(" %-12.2f", calculateGapPercent(exactResult->bestDistance, knownReference));
        }

        printf("\n");
    } else {
        printf("%-30s %-8s %-15s %-15.6f %-12s",
            "Exact Backtracking",
            "SKIPPED",
            "-",
            exactResult->executionTimeSeconds,
            "-"
        );

        if (knownReference > 0.0) {
            printf(" %-12s", "-");
        }

        printf("\n");
    }

    if (knownReference > 0.0) {
        printf("Known reference length: %.2f\n", knownReference);
        printf("Note: known reference values are used only for reporting gap, not for constructing routes.\n");
    } else {
        printf("Known reference length: not available for this input.\n");
    }

    printf("==================================================\n");
}

/*
 * Fonksiyon amacı:
 * Geçerli çözümler arasında en kısa mesafeli olanı seçer.
 *

 */
static int findBestHeuristicIndex(const Solution solutions[], int solutionCount) {
    int bestIndex = -1;
    double bestDistance = INF;

    for (int i = 0; i < solutionCount; i++) {
        if (solutions[i].valid && solutions[i].totalDistance < bestDistance) {
            bestDistance = solutions[i].totalDistance;
            bestIndex = i;
        }
    }

    return bestIndex;
}

/*
 * Fonksiyon amacı:
 * Checker için sade output üretir.
 * İlk satır toplam mesafe, sonraki satırlar route'lardır.
 * Bu modda tablo/debug yazdırılmaz.
 *
 */
static void printPlainOutput(const Problem *problem, const Solution solutions[], int solutionCount, const ExactResult *exactResult) {
    int bestIndex = findBestHeuristicIndex(solutions, solutionCount);

    /*
     * Küçük inputta exact sonuç varsa en güvenilir sonuç olarak onu yazdırıyoruz.
     * Büyük inputlarda exact skipped olur ve en iyi valid heuristic yazdırılır.
     */
    if (exactResult->available) {
        printf("%.6f\n", exactResult->bestDistance);
        for (int i = 0; i < exactResult->routeSize; i++) {
            if (i > 0) {
                printf(" ");
            }
            printf("%d", exactResult->route[i]);
        }
        printf("\n");
        return;
    }

    if (bestIndex < 0) {
        printf("NO VALID SOLUTION\n");
        return;
    }

    printf("%.6f\n", solutions[bestIndex].totalDistance);

    for (int vehicle = 0; vehicle < problem->vehicleCount; vehicle++) {
        for (int position = 0; position < solutions[bestIndex].routeSizes[vehicle]; position++) {
            if (position > 0) {
                printf(" ");
            }
            printf("%d", solutions[bestIndex].routes[vehicle][position]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    /*
     * --plain modu hocanın otomatik checker kullanması ihtimaline karşı sade çıktı üretir.
     * Normal mod ise bizim detaylı analiz ve karşılaştırma raporumuzdur.
     */
    int plainMode = 0;
    if (argc > 1 && strcmp(argv[1], "--plain") == 0) {
        plainMode = 1;
    }

    /*
     * Problem ve Solution büyük structlar içerir.
     * static kullanarak stack overflow riskini azaltıyoruz.
     */
    static Problem problem;
    static Solution solutions[3];
    static ExactResult exactResult;

    readInput(&problem);
    buildDistanceMatrix(&problem);

    runExactBacktrackingForSmallInstance(&problem, &exactResult);

    createSequentialBaseline(&problem, &solutions[0]);
    createNearestNeighborTwoOptSolution(&problem, &solutions[1]);
    createILSTwoOptSolution(&problem, &solutions[2], DEFAULT_ILS_TIME_LIMIT_SECONDS);

    if (plainMode) {
        printPlainOutput(&problem, solutions, 3, &exactResult);
        return 0;
    }

    printInputSummary(&problem);
    printComparisonTable(&problem, solutions, 3, &exactResult);
    printExactResult(&exactResult);

    int bestIndex = findBestHeuristicIndex(solutions, 3);

    if (bestIndex >= 0) {
        printf("\nBest valid heuristic: %s\n", solutions[bestIndex].algorithmName);
        printf("Best heuristic distance: %.6f\n", solutions[bestIndex].totalDistance);
        printValidationDetails(&solutions[bestIndex]);
        printSolutionRoutes(&problem, &solutions[bestIndex]);
    } else {
        printf("\nNo valid heuristic solution found.\n");
    }

    return 0;
}
