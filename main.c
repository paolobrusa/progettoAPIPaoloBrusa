#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    int x, y;
} Coord;

typedef struct Entry {
    Coord key;
    int value;
    struct Entry* next;
} Entry;

typedef struct {
    Entry** buckets;
} HashMapPosition;

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

void delete(HashMapPosition* map, int matrixSize) {
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

HashMapPosition* init(int y, int x, HashMapPosition* map) {
    int matrixSize = y * x;
    if (map != NULL) {
        delete(map, matrixSize);
    }
    return create(matrixSize, x);
}










int main(void) {
    HashMapPosition* map = init(10, 10, NULL);
    int found;
    Coord testKey = {1, 1};
    modifyCoord(map, testKey, 4, 100);
    printf("%d\n",getPair(map, testKey, &found, 100));
    delete(map, 100);
    return 0;
}
