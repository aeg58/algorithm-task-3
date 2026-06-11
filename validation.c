#include <stdio.h>
#include <math.h>
#include "validation.h"
#include "distance.h"

/*
 * Fonksiyon amacı:
 * Her rotanın depot ile başlayıp depot ile bittiğini kontrol eder.
 *
 */
int routesStartAndEndAtDepot(const Problem *problem, const Solution *solution) {
    for (int vehicle = 0; vehicle < problem->vehicleCount; vehicle++) {
        int routeSize = solution->routeSizes[vehicle];

        if (routeSize < 2) {// çünkü en az bir müşteri ve bir depot olmalı
            return 0;
        }

        if (solution->routes[vehicle][0] != DEPOT) { //depotun 0 indexinde olması gerekir
            return 0;
        }

        if (solution->routes[vehicle][routeSize - 1] != DEPOT) { //son indexinde de depot olması gerekir 
            return 0;
        }//routesize-1 son indexi verir çünkü 0 dan başlar
    }
// O(m = vehicleCCount)
    return 1;
}

/*
 * Fonksiyon amacı:
 * Her müşterinin bütün çözümde tam olarak bir kez ziyaret edilip edilmediğini kontrol eder.
 */

int allClientsVisitedExactlyOnce(const Problem *problem, const Solution *solution) {
    int visitCount[MAX_CLIENTS + 1] = {0};// 0 indexi depot için, 1..clientCount arası müşteriler için kullanılır
    

    for (int vehicle = 0; vehicle < problem->vehicleCount; vehicle++) { //tüm routlerı gezeriz
        for (int position = 0; position < solution->routeSizes[vehicle]; position++) {//tüm rotanın tüm noktalarnı gezeriz burdaki değer o aracın rotasındaki nokta sayısıdır
            int vertex = solution->routes[vehicle][position];  //o anki ziyaret edilen nokta (depot veya müşteri)

            if (vertex == DEPOT) {
                continue;//// Depot müşteri değildir ve her route'ta tekrar edebilir.
// Bu yüzden depot'u visitCount dizisine dahil etmiyoruz.
            }

            if (vertex < 1 || vertex > problem->clientCount) {// Geçersiz müşteri numarası
                return 0;
            }

            visitCount[vertex]++;
        }
    }

    for (int client = 1; client <= problem->clientCount; client++) {// Her müşterinin tam olarak bir kez ziyaret edilip edilmediğini kontrol ederiz 
        if (visitCount[client] != 1) {
            return 0; // Eğer herhangi bir müşteri 0 kez veya 1'den fazla kez ziyaret edilmişse, çözüm geçersizdir.
        }
    }

    return 1;
}


 
/*
 * Fonksiyon amacı:
 * Her aracın maxClientsPerVehicle sınırını aşıp aşmadığını kontrol eder.
 * Depot sayılmaz; sadece müşteri sayısı dikkate alınır.
 */ 
int routesRespectMaxClientsPerVehicle(const Problem *problem, const Solution *solution) {
    for (int vehicle = 0; vehicle < problem->vehicleCount; vehicle++) {
        int clientCounter = 0;

        for (int position = 0; position < solution->routeSizes[vehicle]; position++) {
            if (solution->routes[vehicle][position] != DEPOT) {
                clientCounter++;
            }
        }

        if (clientCounter > problem->maxClientsPerVehicle) {
            return 0;
        }
    }

    return 1;
}

/*
 * Fonksiyon amacı:
 * Bütün validation kontrollerini çalıştırır ve total distance değerini bağımsız olarak yeniden hesaplar.
   */
void validateSolution(const Problem *problem, Solution *solution) {
    solution->depotCheck = routesStartAndEndAtDepot(problem, solution);
    solution->clientVisitCheck = allClientsVisitedExactlyOnce(problem, solution);
    solution->capacityCheck = routesRespectMaxClientsPerVehicle(problem, solution);

    solution->recalculatedDistance = calculateTotalDistance(problem, solution);

    /*
     * Heuristic'in kaydettiği mesafe ile bağımsız hesaplanan mesafe karşılaştırılır.
     * Burada küçük floating point farkları için tolerans kullanıyoruz.
     */
    if (fabs(solution->totalDistance - solution->recalculatedDistance) <= 1e-6) {
        solution->distanceCheck = 1;
    } else {
        solution->distanceCheck = 0;
    }

    solution->valid = solution->depotCheck && solution->clientVisitCheck && solution->capacityCheck && solution->distanceCheck;
}

/*
 * Fonksiyon amacı:
 * Validation detaylarını okunabilir şekilde yazdırır.
 */
void printValidationDetails(const Solution *solution) {
    printf("Depot start/end: %s\n", solution->depotCheck ? "VALID" : "INVALID");
    printf("Client visits exactly once: %s\n", solution->clientVisitCheck ? "VALID" : "INVALID");
    printf("Vehicle capacity respected: %s\n", solution->capacityCheck ? "VALID" : "INVALID");
    printf("Stored distance: %.6f\n", solution->totalDistance);
    printf("Recalculated distance: %.6f\n", solution->recalculatedDistance);
    printf("Distance check: %s\n", solution->distanceCheck ? "OK" : "MISMATCH");
}
