#ifndef COMMON_H    
#define COMMON_H //birden fazla include edilip tekrar tanımlanmayı engellemek için include guard

/*
 * common.h
 *
 * Bu dosyada bütün kaynak dosyaların ortak kullandığı sabitler ve struct yapıları bulunur.
 * Amaç: main.c, input.c, distance.c, validation.c, heuristic.c ve exact.c dosyalarının
 * aynı veri tiplerini kullanmasını sağlamaktır.
 */

#define MAX_CLIENTS 1000
#define MAX_VEHICLES 10
#define MAX_ROUTE_SIZE (MAX_CLIENTS + 2)
#define DEPOT 0 // örnekte 0 dan baslandıgı icin depot 0 olarak tanımlandı
#define INF 1e100 //sonsuz değer
#define EXACT_MAX_CLIENTS 11 //backtracking O(n!) olduğundan 11 müşteriden fazlası için çalıştırılmayacak Bu yüzden büyük inputlarda heuristic kullanıyoruz.
#define DEFAULT_ILS_TIME_LIMIT_SECONDS 2.0 //çalışma süresini kontrol altında tutmak ve beklenen süre limitinin altında kalmak için sabit süre limiti kullandım.

/*
 * Problem struct'ı inputtan okunan bütün problem verisini tutar.
 * 0 indexi depot, 1..clientCount arası müşterilerdir.
 */
typedef struct {  
    int clientCount;
    int vehicleCount;
    int maxClientsPerVehicle;
    double xCoord[MAX_CLIENTS + 1]; // 0 indexi depot, 1..clientCount arası müşterilerdir.  
    double yCoord[MAX_CLIENTS + 1];
    double distanceMatrix[MAX_CLIENTS + 1][MAX_CLIENTS + 1];
} Problem;

/*
 * Solution struct'ı bir algoritmanın ürettiği çözümü tutar.
 * routes[vehicle][position] yapısı, her aracın rotasını temsil eder.
 */
typedef struct {
    int routes[MAX_VEHICLES][MAX_ROUTE_SIZE];
    int routeSizes[MAX_VEHICLES];
    double totalDistance;
    double recalculatedDistance;
    double executionTimeSeconds;
    int valid;
    int depotCheck;
    int clientVisitCheck;
    int capacityCheck;
    int distanceCheck;
    char algorithmName[100];
} Solution;

/*
 * ExactResult küçük inputlar için exact backtracking sonucunu tutar.
 * Büyük inputlarda exact çalıştırılmaz, skipped = 1 olur.
 */
typedef struct {
    int available;
    int skipped;
    double bestDistance;
    double executionTimeSeconds;
    int route[MAX_ROUTE_SIZE];
    int routeSize;
    char message[160];
} ExactResult;

#endif
