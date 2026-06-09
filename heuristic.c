#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "heuristic.h"
#include "distance.h"
#include "validation.h"

/*
 * Fonksiyon amacı:
 * Solution struct'ını temizler ve algoritma adını yazar.
 *

 */
static void initializeSolution(Solution *solution, const char *algorithmName) {
    memset(solution, 0, sizeof(Solution));
    snprintf(solution->algorithmName, sizeof(solution->algorithmName), "%s", algorithmName);
}

/*
 * Fonksiyon amacı:
 * Solution kopyalamak için kullanılır.
 */
static void copySolution(Solution *destination, const Solution *source) {
    memcpy(destination, source, sizeof(Solution));
}

/*
 * Fonksiyon amacı:
 * Her aracın rotasını depot ile başlatır.
 *
 */
static void startAllRoutesAtDepot(const Problem *problem, Solution *solution) {
    for (int vehicle = 0; vehicle < problem->vehicleCount; vehicle++) {
        solution->routes[vehicle][0] = DEPOT;
        solution->routeSizes[vehicle] = 1;
    }
}

/*
 * Fonksiyon amacı:
 * Her aracın rotasını depot ile kapatır.
 *
 */
static void closeAllRoutesWithDepot(const Problem *problem, Solution *solution) {
    for (int vehicle = 0; vehicle < problem->vehicleCount; vehicle++) {
        int size = solution->routeSizes[vehicle];
        solution->routes[vehicle][size] = DEPOT;
        solution->routeSizes[vehicle]++;
    }
}

/*
 * Fonksiyon amacı:
 * Solution mesafesini hesaplar ve validation çalıştırır.
 *
 */
static void finalizeSolution(const Problem *problem, Solution *solution) {
    solution->totalDistance = calculateTotalDistance(problem, solution);
    validateSolution(problem, solution);
}

/*
 * Fonksiyon amacı:
 * Basit sequential baseline çözüm üretir. Müşteriler sırayla araçlara dağıtılır.
 * Bu optimal değildir; sadece karşılaştırma için basit referans noktasıdır.

 */
void createSequentialBaseline(const Problem *problem, Solution *solution) {
    clock_t start = clock();
    initializeSolution(solution, "Naive Sequential Baseline");
    startAllRoutesAtDepot(problem, solution);

    int currentClient = 1;

    for (int vehicle = 0; vehicle < problem->vehicleCount && currentClient <= problem->clientCount; vehicle++) {
        int addedClients = 0;

        while (addedClients < problem->maxClientsPerVehicle && currentClient <= problem->clientCount) {
            int position = solution->routeSizes[vehicle];
            solution->routes[vehicle][position] = currentClient;
            solution->routeSizes[vehicle]++;
            currentClient++;
            addedClients++;
        }
    }

    closeAllRoutesWithDepot(problem, solution);
    finalizeSolution(problem, solution);
    solution->executionTimeSeconds = (double)(clock() - start) / CLOCKS_PER_SEC;
}

/*
 * Fonksiyon amacı:
 * Mevcut noktaya en yakın ziyaret edilmemiş müşteriyi bulur.
 *
 */
static int findNearestUnvisitedClient(const Problem *problem, int currentVertex, const int visited[]) {
    int nearestClient = -1;
    double bestDistance = INF;

    for (int client = 1; client <= problem->clientCount; client++) {
        if (visited[client]) {
            continue;
        }

        double distance = problem->distanceMatrix[currentVertex][client];
        if (distance < bestDistance) {
            bestDistance = distance;
            nearestClient = client;
        }
    }

    return nearestClient;
}

/*
 * Fonksiyon amacı:
 * Randomized nearest neighbor için en yakın birkaç aday arasından rastgele seçim yapar.
 * Bu ILS içinde farklı başlangıç çözümleri üretmek için kullanılır.
 *
 */
static int findRandomNearUnvisitedClient(const Problem *problem, int currentVertex, const int visited[], int candidateLimit) {
    int candidates[20];
    double candidateDistances[20];
    int candidateCount = 0;

    if (candidateLimit > 20) {
        candidateLimit = 20;
    }

    /*
     * Küçük bir sorted candidate list tutuyoruz.
     * Amaç: tamamen rastgele değil, yakın müşteriler arasında yarı-rastgele seçim yapmak.
     */
    for (int client = 1; client <= problem->clientCount; client++) {
        if (visited[client]) {
            continue;
        }

        double distance = problem->distanceMatrix[currentVertex][client];
        int insertPosition = candidateCount;

        while (insertPosition > 0 && candidateDistances[insertPosition - 1] > distance) {
            if (insertPosition < candidateLimit) {
                candidateDistances[insertPosition] = candidateDistances[insertPosition - 1];
                candidates[insertPosition] = candidates[insertPosition - 1];
            }
            insertPosition--;
        }

        if (insertPosition < candidateLimit) {
            candidateDistances[insertPosition] = distance;
            candidates[insertPosition] = client;
            if (candidateCount < candidateLimit) {
                candidateCount++;
            }
        }
    }

    if (candidateCount == 0) {
        return -1;
    }

    return candidates[rand() % candidateCount];
}

/*
 * Fonksiyon amacı:
 * Bir rotaya first-improvement 2-opt uygular.
 * İki kenar değiştirilince rota kısalıyorsa değişiklik kabul edilir.
 */
static void applyTwoOptToRoute(const Problem *problem, int route[], int routeSize) {
    int improved = 1;

    while (improved) {
        improved = 0;

        for (int i = 1; i < routeSize - 2 && !improved; i++) {
            for (int j = i + 1; j < routeSize - 1 && !improved; j++) {
                int a = route[i - 1];
                int b = route[i];
                int c = route[j];
                int d = route[j + 1];

                double oldCost = problem->distanceMatrix[a][b] + problem->distanceMatrix[c][d];
                double newCost = problem->distanceMatrix[a][c] + problem->distanceMatrix[b][d];

                if (newCost + 1e-9 < oldCost) {
                    int left = i;
                    int right = j;

                    while (left < right) {
                        int temp = route[left];
                        route[left] = route[right];
                        route[right] = temp;
                        left++;
                        right--;
                    }

                    improved = 1;
                }
            }
        }
    }
}

/*
 * Fonksiyon amacı:
 * Bütün araç rotalarına 2-opt uygular.
 *
 * English explanation:
 * This helper applies 2-opt improvement to every route.
 */
static void applyTwoOptToAllRoutes(const Problem *problem, Solution *solution) {
    for (int vehicle = 0; vehicle < problem->vehicleCount; vehicle++) {
        applyTwoOptToRoute(problem, solution->routes[vehicle], solution->routeSizes[vehicle]);
    }
}

/*
 * Fonksiyon amacı:
 * Deterministic nearest neighbor çözümü üretir. */
static void createNearestNeighborRaw(const Problem *problem, Solution *solution) {
    startAllRoutesAtDepot(problem, solution);

    int visited[MAX_CLIENTS + 1] = {0};
    int visitedCount = 0;

    for (int vehicle = 0; vehicle < problem->vehicleCount && visitedCount < problem->clientCount; vehicle++) {
        int currentVertex = DEPOT;
        int addedClients = 0;

        while (addedClients < problem->maxClientsPerVehicle && visitedCount < problem->clientCount) {
            int nearestClient = findNearestUnvisitedClient(problem, currentVertex, visited);

            if (nearestClient == -1) {
                break;
            }

            int position = solution->routeSizes[vehicle];
            solution->routes[vehicle][position] = nearestClient;
            solution->routeSizes[vehicle]++;

            visited[nearestClient] = 1;
            visitedCount++;
            addedClients++;
            currentVertex = nearestClient;
        }
    }

    closeAllRoutesWithDepot(problem, solution);
}

/*
 * Fonksiyon amacı:
 * Nearest Neighbor + 2-opt heuristic üretir.
 *
 */
void createNearestNeighborTwoOptSolution(const Problem *problem, Solution *solution) {
    clock_t start = clock();
    initializeSolution(solution, "Nearest Neighbor + 2-opt");

    createNearestNeighborRaw(problem, solution);
    applyTwoOptToAllRoutes(problem, solution);
    finalizeSolution(problem, solution);

    solution->executionTimeSeconds = (double)(clock() - start) / CLOCKS_PER_SEC;
}


/*
 * Fonksiyon amacı:
 * İki farklı route arasında müşteri swap işleminin mesafe değişimini hesaplar.
 * Bu sadece farklı araç rotaları için kullanılır.
 *
 */
static double calculateInterRouteSwapDelta(const Problem *problem, const Solution *solution, int vehicleA, int posA, int vehicleB, int posB) {
    int clientA = solution->routes[vehicleA][posA];
    int clientB = solution->routes[vehicleB][posB];

    int prevA = solution->routes[vehicleA][posA - 1];
    int nextA = solution->routes[vehicleA][posA + 1];
    int prevB = solution->routes[vehicleB][posB - 1];
    int nextB = solution->routes[vehicleB][posB + 1];

    double oldCost = 0.0;
    oldCost += problem->distanceMatrix[prevA][clientA] + problem->distanceMatrix[clientA][nextA];
    oldCost += problem->distanceMatrix[prevB][clientB] + problem->distanceMatrix[clientB][nextB];

    double newCost = 0.0;
    newCost += problem->distanceMatrix[prevA][clientB] + problem->distanceMatrix[clientB][nextA];
    newCost += problem->distanceMatrix[prevB][clientA] + problem->distanceMatrix[clientA][nextB];

    return newCost - oldCost;
}

/*
 * Fonksiyon amacı:
 * Çok araçlı çözümlerde farklı araç rotaları arasında müşteri swap local search uygular.
 * Bu işlem, ILS'in sadece route içini değil araçlar arası müşteri dağılımını da iyileştirmesini sağlar.
  */
static void applyInterRouteSwapLocalSearch(const Problem *problem, Solution *solution) {
    if (problem->vehicleCount < 2) {
        return;
    }

    int improved = 1;

    while (improved) {
        improved = 0;

        for (int vehicleA = 0; vehicleA < problem->vehicleCount && !improved; vehicleA++) {
            for (int vehicleB = vehicleA + 1; vehicleB < problem->vehicleCount && !improved; vehicleB++) {
                for (int posA = 1; posA < solution->routeSizes[vehicleA] - 1 && !improved; posA++) {
                    for (int posB = 1; posB < solution->routeSizes[vehicleB] - 1 && !improved; posB++) {
                        double delta = calculateInterRouteSwapDelta(problem, solution, vehicleA, posA, vehicleB, posB);

                        if (delta < -1e-9) {
                            int temp = solution->routes[vehicleA][posA];
                            solution->routes[vehicleA][posA] = solution->routes[vehicleB][posB];
                            solution->routes[vehicleB][posB] = temp;

                            /*
                             * Swap sonrası sadece etkilenen iki route'a tekrar 2-opt uyguluyoruz.
                             * Bu, route içi sıralamayı yeniden toparlar.
                             */
                            applyTwoOptToRoute(problem, solution->routes[vehicleA], solution->routeSizes[vehicleA]);
                            applyTwoOptToRoute(problem, solution->routes[vehicleB], solution->routeSizes[vehicleB]);

                            improved = 1;
                        }
                    }
                }
            }
        }
    }
}

/*
 * Fonksiyon amacı:
 * ILS için yarı-rastgele bir başlangıç çözümü üretir.
 *
 */
static void createSemiRandomSolution(const Problem *problem, Solution *solution, int candidateLimit) {
    startAllRoutesAtDepot(problem, solution);

    int visited[MAX_CLIENTS + 1] = {0};
    int visitedCount = 0;

    for (int vehicle = 0; vehicle < problem->vehicleCount && visitedCount < problem->clientCount; vehicle++) {
        int currentVertex = DEPOT;
        int addedClients = 0;

        while (addedClients < problem->maxClientsPerVehicle && visitedCount < problem->clientCount) {
            int selectedClient = findRandomNearUnvisitedClient(problem, currentVertex, visited, candidateLimit);

            if (selectedClient == -1) {
                break;
            }

            int position = solution->routeSizes[vehicle];
            solution->routes[vehicle][position] = selectedClient;
            solution->routeSizes[vehicle]++;

            visited[selectedClient] = 1;
            visitedCount++;
            addedClients++;
            currentVertex = selectedClient;
        }
    }

    closeAllRoutesWithDepot(problem, solution);
}


/*
 * Fonksiyon amacı:
 * ILS içinde mevcut iyi çözümü bozmadan küçük rastgele değişiklikler yapar.
 * Tek araçta rota içinde bir parçayı ters çevirir.
 * Birden fazla araçta ise farklı araçlar arasında müşteri değişimi yapar.
  */
static void perturbSolution(const Problem *problem, Solution *solution, int strength) {
    if (strength < 1) {
        strength = 1;
    }

    for (int step = 0; step < strength; step++) {
        if (problem->vehicleCount == 1) {
            int routeSize = solution->routeSizes[0];

            /*
             * Rota içinde depot dışındaki iki pozisyon seçiyoruz.
             * Bu iki pozisyon arasındaki parçayı ters çevirerek yeni bir çözüm deniyoruz.
             */
            if (routeSize > 5) {
                int left = 1 + rand() % (routeSize - 3);
                int right = 1 + rand() % (routeSize - 3);

                if (left > right) {
                    int temp = left;
                    left = right;
                    right = temp;
                }

                while (left < right) {
                    int temp = solution->routes[0][left];
                    solution->routes[0][left] = solution->routes[0][right];
                    solution->routes[0][right] = temp;
                    left++;
                    right--;
                }
            }
        } else {
            int vehicleA = rand() % problem->vehicleCount;
            int vehicleB = rand() % problem->vehicleCount;

            if (vehicleA == vehicleB) {
                vehicleB = (vehicleB + 1) % problem->vehicleCount;
            }

            int sizeA = solution->routeSizes[vehicleA];
            int sizeB = solution->routeSizes[vehicleB];

            /*
             * Depot pozisyonları 0 ve size-1 olduğu için sadece 1..size-2 arasından müşteri seçiyoruz.
             * İki müşteri yer değiştirince araç başına müşteri sayısı değişmez, yani kapasite bozulmaz.
             */
            if (sizeA > 2 && sizeB > 2) {
                int posA = 1 + rand() % (sizeA - 2);
                int posB = 1 + rand() % (sizeB - 2);

                int temp = solution->routes[vehicleA][posA];
                solution->routes[vehicleA][posA] = solution->routes[vehicleB][posB];
                solution->routes[vehicleB][posB] = temp;
            }
        }
    }
}

/*
 * Fonksiyon amacı:
 * Iterated Local Search + 2-opt çalıştırır.
 * Süre dolana kadar çözüm üretir, 2-opt ile iyileştirir ve en iyisini saklar.
 *
 */
void createILSTwoOptSolution(const Problem *problem, Solution *solution, double timeLimitSeconds) {
    clock_t start = clock();
    initializeSolution(solution, "ILS + 2-opt");

    Solution best;
    Solution current;

    /*
     * Başlangıç olarak NN + 2-opt sonucunu kullanıyoruz.
     * Böylece ILS en başta zaten makul kaliteli bir çözümle başlar.
     */
    createNearestNeighborTwoOptSolution(problem, &best);
    snprintf(best.algorithmName, sizeof(best.algorithmName), "%s", "ILS + 2-opt");

    srand(42);

    int iteration = 0;
    while (((double)(clock() - start) / CLOCKS_PER_SEC) < timeLimitSeconds) {
        if (iteration % 3 == 0) {
            /*
             * Bazı iterasyonlarda tamamen yeni yarı-rastgele çözüm kuruyoruz.
             * Bu, farklı müşteri sıralamaları denememizi sağlar.
             */
            initializeSolution(&current, "ILS candidate");
            int candidateLimit = 4 + (iteration % 8);
            createSemiRandomSolution(problem, &current, candidateLimit);
        } else {
            /*
             * Diğer iterasyonlarda mevcut en iyi çözümü küçük değişikliklerle bozuyoruz.
             * Çok araçlı durumda özellikle rotalar arası müşteri swap işlemi ILS'i NN'den farklı hale getirir.
             */
            copySolution(&current, &best);
            int strength = 1 + (iteration % 5);
            perturbSolution(problem, &current, strength);
        }

        applyTwoOptToAllRoutes(problem, &current);
        applyInterRouteSwapLocalSearch(problem, &current);
        finalizeSolution(problem, &current);

        if (current.valid && current.totalDistance < best.totalDistance) {
            copySolution(&best, &current);
            snprintf(best.algorithmName, sizeof(best.algorithmName), "%s", "ILS + 2-opt");
        }

        iteration++;
    }

    copySolution(solution, &best);
    snprintf(solution->algorithmName, sizeof(solution->algorithmName), "%s", "ILS + 2-opt");
    solution->executionTimeSeconds = (double)(clock() - start) / CLOCKS_PER_SEC;
}

/*
 * Fonksiyon amacı:
 * optimal_solutions.pdf dosyasında verilen bilinen referans değerleri döndürür.
 * Bu değerler algoritma içinde kullanılmaz; sadece gap raporlamak için kullanılır.
 *
 */
double getKnownReferenceLength(const Problem *problem) {
    if (problem->clientCount == 400 && problem->vehicleCount == 1 && problem->maxClientsPerVehicle == 400) {
        return 15300.0;
    }

    if (problem->clientCount == 400 && problem->vehicleCount == 2 && problem->maxClientsPerVehicle == 200) {
        return 15500.0;
    }

    return -1.0;
}

/*
 * Fonksiyon amacı:
 * Bir çözümün rotalarını yazdırır.
 *
 */
void printSolutionRoutes(const Problem *problem, const Solution *solution) {
    printf("\nBest solution routes (%s):\n", solution->algorithmName);

    for (int vehicle = 0; vehicle < problem->vehicleCount; vehicle++) {
        printf("Route %d:", vehicle + 1);

        for (int position = 0; position < solution->routeSizes[vehicle]; position++) {
            printf(" %d", solution->routes[vehicle][position]);
        }

        printf("\n");
    }
}
