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
void buildDistanceMatrix(Problem *problem) {//Öklid mesafelerini tekrar tekrar hesaplamamak için distance matrix oluşturuyorum. Ön hesaplamadan sonra her mesafe erişimi O(1) olur.
    int totalPoints = problem->clientCount + 1; // 1 fazla depot için
//
    for (int i = 0; i < totalPoints; i++) {
        for (int j = 0; j < totalPoints; j++) {
            problem->distanceMatrix[i][j] = calculateEuclideanDistance(
                problem->xCoord[i], problem->yCoord[i],
                problem->xCoord[j], problem->yCoord[j]
            );
        }
    }
}//time comp 0(n^2)

/*
 * Fonksiyon amacı:
 * Tek bir rotanın toplam mesafesini bağımsız şekilde hesaplar.
 *

 */
double calculateRouteDistance(const Problem *problem, const int route[], int routeSize) {
    double routeDistance = 0.0;

    for (int position = 0; position < routeSize - 1; position++) { //routeSize-1 çünkü son noktadan sonra başka bir nokta yok, yani routeSize-1 tane kenar var.
        int from = route[position];
        int to = route[position + 1];
        routeDistance += problem->distanceMatrix[from][to];
    }

    return routeDistance;
}//Bu fonksiyon rota içindeki ardışık noktaları gezer ve distance matrix kullanarak her ardışık nokta çiftinin mesafesini toplar.

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
}//Bu fonksiyon validation sırasında çözümün toplam mesafesini bağımsız olarak yeniden hesaplamak için de kullanılır.
//Time complexity: O(n + m)

//The input gives coordinates for clients and the depot, so I calculate straight-line Euclidean distances between these coordinate points.
 

//Kaydedilen mesafenin gerçek rota ile uyuştuğunu doğrulamak için. Böylece heuristic’e körü körüne güvenmemiş oluyoruz.