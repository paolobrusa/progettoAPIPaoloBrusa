#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    int x, y;
} Coord;

typedef struct {
    Coord value1;
    int routeCost;
} valueAir;

typedef struct Entry {
    Coord key;
    int value;
    struct Entry* next;
} Entry;

typedef struct {
    Entry** buckets;
} HashMapPosition;

typedef struct EntryAiroute {
    Coord key;
    valueAir value[5];
    struct EntryAiroute* next;
} EntryAiroute;

typedef struct {
    EntryAiroute** bucketsAir;
} HashMapAiroute;

//Funzione hash
unsigned int hashing(Coord coord, int matrixSize) {
    unsigned int hash = coord.x * 31 + coord.y;
    return hash % matrixSize;
}

int checkKey(Coord a, Coord b) {
    return (a.x == b.x) && (a.y == b.y);
}

HashMapPosition* create(int matrixSize, int raw) {
    int j = 0, k = 0;
    HashMapPosition* map = malloc(sizeof(HashMapPosition));
    map->buckets = calloc(matrixSize, sizeof(Entry*));
    for (int i = 0; i < matrixSize; i++) {
        Coord key = {j,k};
        unsigned int index = hashing(key, matrixSize);
        Entry* entry = malloc(sizeof(Entry));
        entry->key = key;
        entry->value = 1;
        entry->next = map->buckets[index];
        map->buckets[index] = entry;
        j++;
        if (j == raw) {
            j = 0;                              //POSSIBILE BUG SE HAI GESTITO MALE RIGHE/COLONNE
            k++;
        }
    }
    return map;
}

HashMapAiroute* create2(int matrixSize) {
    HashMapAiroute* map = malloc(sizeof(HashMapAiroute));
    map->bucketsAir = calloc(matrixSize/2, sizeof(EntryAiroute*));
    return map;
}

void insertRoute(HashMapAiroute* map, Coord key, Coord value, int value2, int matrixSize) {
    unsigned int index = hashing(key, matrixSize/2);
    EntryAiroute* entry = map->bucketsAir[index];
    while (entry != NULL) {
        if (checkKey(entry->key, key)) {
            for (int i = 0; i < 5; i++) {
                if (checkKey(entry->value[i].value1, value)) {
                    entry->value[i].routeCost = value2;
                    return;
                }
                //cosa ci sara dentro entry value?
            }
        }
        entry = entry->next;
    }
    entry = malloc(sizeof(EntryAiroute));
    entry->key = key;
    for (int i = 0; i < 5; i++) {
        entry->value[i].value1 = value;
        entry->value[i].routeCost = value2;
    }
    entry->next = map->bucketsAir[index];
    map->bucketsAir[index] = entry;
}

void modifyCoord(HashMapPosition* map, Coord key, int value, int matrixSize) {
    unsigned int index = hashing(key, matrixSize);
    Entry* entry = map->buckets[index];
    while (entry != NULL) {
        if (checkKey(entry->key, key)) {
            entry->value = value;
            return;
        }
        entry = entry->next;
    }
}

int getPair(HashMapPosition* map, Coord key, int* found, int matrixSize) {
    unsigned int index = hashing(key, matrixSize);
    Entry* entry = map->buckets[index];
    while (entry != NULL) {
        if (checkKey(entry->key, key)) {
            *found = 1;                                                      //RICONTROLLA QUI
            return entry->value;
        }
        entry = entry->next;
    }
    *found = 0;
    return 0;
}

void deletePos(HashMapPosition* map, int matrixSize) {
    for (int i = 0; i < matrixSize; i++) {
        Entry* entry = map->buckets[i];
        while (entry != NULL) {
            Entry* next = entry->next;
            free(entry);
            entry = next;
        }
    }
    free(map->buckets);
    free(map);
}

void deleteAiroute(HashMapAiroute* map, int matrixSize) {
    for (int i = 0; i < matrixSize/2; i++) {
        EntryAiroute* entry = map->bucketsAir[i];
        while (entry != NULL) {
            EntryAiroute* next = entry->next;
            free(entry);
            entry = next;
        }
    }
    free(map->bucketsAir);
    free(map);
}













HashMapPosition* init(int y, int x, HashMapPosition* map, int matrixSizeOld) {
    int matrixSize = y * x;
    if (map != NULL) {
        deletePos(map, matrixSizeOld);
    }
    return create(matrixSize, x);
}










int main(int argc, const char *argv[]) {
    HashMapPosition* map = NULL;
    int x, y, precX, precY;
    while (1) {
        precX = x;
        precY = y;
        scanf("%d %d", &x, &y);
        if (x == -1 && y == -1) {
            break;
        }
        map = init(y, x, map, precX * precY);
        int found = 0;
        Coord key = {1, 1};
        int value = getPair(map, key, &found, x * y);
        printf("%d\n", value);
    }
    deletePos(map, precX * precY);
    return 0;
}
