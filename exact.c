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
//bu değişkenlerin sadece exact.c dosyası içinde kullanılmasını sağlar.
static double bestDistance;
static int bestRoute[MAX_ROUTE_SIZE];
static int currentRoute[MAX_ROUTE_SIZE];
static int visited[MAX_CLIENTS + 1];

/*
 * Fonksiyon amacı:
 * TSP backtracking recursive araması yapar.
 *

 */ 

 // bizim exact algorith mamız sadece 1 araçlı ve müşteri sayısı 11 veya daha azsa çalışıyor
static void exactBacktrack(const Problem *problem, int level, int currentVertex, double currentDistance) {
    if (currentDistance >= bestDistance) {
        return;
    }

    if (level == problem->clientCount) {//tüm müşteriler ziyaret edildiyse depoya dön ve toplam mesafeyi hesapla level şuanakı ziyaret edilen müşteri sayısını tutuyor
        double totalDistance = currentDistance + problem->distanceMatrix[currentVertex][DEPOT]; //şu ana kadar gidilen mesafe + depoya dönüş mesafesi

        if (totalDistance < bestDistance) {
            bestDistance = totalDistance;// daha iyisi bulunduğunda güncelle


            for (int i = 0; i <= problem->clientCount; i++) { //mevcut rotayı en iyi rota olarak kaydet
                bestRoute[i] = currentRoute[i];
            }

            bestRoute[problem->clientCount + 1] = DEPOT;
        }

        return;
    }

    for (int client = 1; client <= problem->clientCount; client++) {
        if (!visited[client]) {
            visited[client] = 1;
            currentRoute[level + 1] = client; //level 0'da depoya başlıyoruz, level 1'de ilk müşteri, level 2'de ikinci müşteri şeklinde ilerliyoruz

            exactBacktrack(
                problem,
                level + 1, //bir müşteri daha eklendiği için level'i artır
                client,
                currentDistance + problem->distanceMatrix[currentVertex][client] //şu ana kadar gidilen mesafeye yeni müşteriye gitme mesafesini ekle
            );

            visited[client] = 0;
        }
    } //time comp 0(n!) çünkü tüm permütasyonları deniyoruz
}

/*
 * Fonksiyon amacı:
 * Küçük input için exact backtracking solver çalıştırır.
 * Büyük inputlarda bilinçli olarak SKIPPED sonucu üretir.
 *

 */
void runExactBacktrackingForSmallInstance(const Problem *problem, ExactResult *result) {
    memset(result, 0, sizeof(ExactResult)); //sonuç struct'ını sıfırla ve default değerler ata
    clock_t start = clock(); //zaman ölçümüne başla

    if (problem->vehicleCount != 1) { //birden fazla araç varsa exact çözüm uygulanmaz çünkü algoritmamız sadece tek araçlı durum için tasarlandı
        result->skipped = 1;
        snprintf(result->message, sizeof(result->message),
                 "SKIPPED: exact backtracking is implemented only for one-vehicle small instances.");
        result->executionTimeSeconds = (double)(clock() - start) / CLOCKS_PER_SEC;
        return;
    }
 
    if (problem->clientCount > EXACT_MAX_CLIENTS) { //müşteri sayısı belirli bir sınırın üzerindeyse exact çözüm uygulanmaz çünkü algoritmanın zaman karmaşıklığı çok yüksek olur
        result->skipped = 1;
        snprintf(result->message, sizeof(result->message),
                 "SKIPPED: instance too large for exact factorial search (clientCount > %d).", EXACT_MAX_CLIENTS);
        result->executionTimeSeconds = (double)(clock() - start) / CLOCKS_PER_SEC;
        return;
    }

    if (problem->maxClientsPerVehicle < problem->clientCount) { //araç kapasitesi tüm müşterileri tek araçla servis etmeye yetmiyorsa exact çözüm uygulanmaz çünkü algoritmamız tek araçlı durum için tasarlandı
        result->skipped = 1;
        snprintf(result->message, sizeof(result->message),
                 "SKIPPED: one vehicle cannot serve all clients because of capacity limit.");
        result->executionTimeSeconds = (double)(clock() - start) / CLOCKS_PER_SEC;
        return;
    }

    bestDistance = INF; //başlangıçta en iyi mesafe sonsuz olarak ayarlanır, böylece ilk bulunan geçerli rota otomatik olarak en iyi rota olur
    memset(visited, 0, sizeof(visited));
    memset(bestRoute, 0, sizeof(bestRoute));
    memset(currentRoute, 0, sizeof(currentRoute));

    currentRoute[0] = DEPOT;
    exactBacktrack(problem, 0, DEPOT, 0.0);
    //exactBacktrack tamamlandıktan sonra bestDistance ve bestRoute en iyi bulunan çözümü içerir. Sonuç struct'ına bu bilgileri kaydet.
    result->available = 1;
    result->bestDistance = bestDistance;
    result->routeSize = problem->clientCount + 2;
 
    for (int i = 0; i < result->routeSize; i++) {
        result->route[i] = bestRoute[i];
    }//rota bilgilerini sonuç struct'ına kopyala

    snprintf(result->message, sizeof(result->message), "Exact backtracking completed for small instance.");
    result->executionTimeSeconds = (double)(clock() - start) / CLOCKS_PER_SEC; //exactBacktrack fonksiyonunun çalışma süresini hesapla ve sonuç struct'ına kaydet
}
//time comp 0(n!) çünkü tüm permütasyonları deniyoruz, ancak bu sadece küçük inputlarda çalıştırılır, büyük inputlarda SKIPPED sonucu üretilir.
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
/*Exact modülü küçük tek araçlı inputlar için backtracking uygular. 
Tüm olası müşteri sıralamalarını recursive şekilde kurar, en iyi tam rotayı saklar ve mevcut kısmi rota en iyi bilinen rotadan uzunsa budama yapar. 
400 müşteri gibi büyük inputlarda exact search bilinçli olarak atlanır çünkü en kötü durum karmaşıklığı faktöriyeldir, O(n!). 
Bu yüzden büyük testlerde heuristic kullanılır.
*/