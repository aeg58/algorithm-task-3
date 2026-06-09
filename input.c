#include <stdio.h>
#include <stdlib.h>
#include "input.h"

/*
 * Fonksiyon amacı:
 * Hocanın verdiği input formatını okur.
 *
 * Format:
 * clientCount vehicleCount maxClientsPerVehicle
 * client1_x client1_y
 * ...
 * clientN_x clientN_y
 * depot_x depot_y
 *

 */
void readInput(Problem *problem) {
    if (scanf("%d %d %d", &problem->clientCount, &problem->vehicleCount, &problem->maxClientsPerVehicle) != 3) {
        printf("Input okuma hatasi: ilk satir okunamadi.\n");
        printf("Beklenen format: clientCount vehicleCount maxClientsPerVehicle\n");
        exit(1);
    }

    if (problem->clientCount < 1 || problem->clientCount > MAX_CLIENTS) {
        printf("Input hatasi: clientCount 1 ile %d arasinda olmali.\n", MAX_CLIENTS);
        exit(1);
    }

    if (problem->vehicleCount < 1 || problem->vehicleCount > MAX_VEHICLES) {
        printf("Input hatasi: vehicleCount 1 ile %d arasinda olmali.\n", MAX_VEHICLES);
        exit(1);
    }

    if (problem->maxClientsPerVehicle < 1 || problem->maxClientsPerVehicle > MAX_CLIENTS) {
        printf("Input hatasi: maxClientsPerVehicle gecersiz.\n");
        exit(1);
    }

    /*
     * Müşteriler kod içinde 1'den başlar.
     * 0 indexi depot olduğu için müşteri koordinatlarını 1..clientCount aralığına kaydediyoruz.
     */
    for (int client = 1; client <= problem->clientCount; client++) {
        if (scanf("%lf %lf", &problem->xCoord[client], &problem->yCoord[client]) != 2) {
            printf("Input okuma hatasi: client %d koordinati okunamadi.\n", client);
            exit(1);
        }
    }

    /*
     * Inputta en son satır depot koordinatıdır.
     * Kod içinde depot'u 0 indexine kaydediyoruz.
     */
    if (scanf("%lf %lf", &problem->xCoord[DEPOT], &problem->yCoord[DEPOT]) != 2) {
        printf("Input okuma hatasi: depot koordinati okunamadi.\n");
        exit(1);
    }
}

/*
 * Fonksiyon amacı:
 * Input doğru okunmuş mu diye kısa bir özet yazdırır.
 *


 */
void printInputSummary(const Problem *problem) {
    printf("Task 3 - Vehicle Routing Problem / Heuristic Assignment\n");
    printf("Clients: %d\n", problem->clientCount);
    printf("Vehicles: %d\n", problem->vehicleCount);
    printf("Max clients per vehicle: %d\n", problem->maxClientsPerVehicle);
    printf("Depot coordinate: %.2f %.2f\n", problem->xCoord[DEPOT], problem->yCoord[DEPOT]);
}
