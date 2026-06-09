#include <math.h>
#include "distance.h"

/*
 * Fonksiyon amacı:
 * İki koordinat arasındaki Öklid mesafesini hesaplar.
 *

 */
double calculateEuclideanDistance(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqrt(dx * dx + dy * dy);
}

/*
 * Fonksiyon amacı:
 * Tüm noktalar arasındaki mesafeleri önceden hesaplayıp distanceMatrix içine yazar.
 *

 */
void buildDistanceMatrix(Problem *problem) {
    int totalPoints = problem->clientCount + 1;

    for (int i = 0; i < totalPoints; i++) {
        for (int j = 0; j < totalPoints; j++) {
            problem->distanceMatrix[i][j] = calculateEuclideanDistance(
                problem->xCoord[i], problem->yCoord[i],
                problem->xCoord[j], problem->yCoord[j]
            );
        }
    }
}

/*
 * Fonksiyon amacı:
 * Tek bir rotanın toplam mesafesini bağımsız şekilde hesaplar.
 *

 */
double calculateRouteDistance(const Problem *problem, const int route[], int routeSize) {
    double routeDistance = 0.0;

    for (int position = 0; position < routeSize - 1; position++) {
        int from = route[position];
        int to = route[position + 1];
        routeDistance += problem->distanceMatrix[from][to];
    }

    return routeDistance;
}

/*
 * Fonksiyon amacı:
 * Çözümdeki bütün araç rotalarının toplam mesafesini bağımsız olarak hesaplar.
 *

 */
double calculateTotalDistance(const Problem *problem, const Solution *solution) {
    double totalDistance = 0.0;

    for (int vehicle = 0; vehicle < problem->vehicleCount; vehicle++) {
        totalDistance += calculateRouteDistance(problem, solution->routes[vehicle], solution->routeSizes[vehicle]);
    }

    return totalDistance;
}
