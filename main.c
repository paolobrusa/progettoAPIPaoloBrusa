#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    int x, y;
} Coord;

typedef struct ValueAir{
    Coord value1;
    int routeCost;
    struct ValueAir* next;
} ValueAir;

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
    ValueAir *value;
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

float getPair(HashMapPosition* map, Coord key, int matrixSize);

HashMapPosition* init(int matrixSize, int raw) {
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
    int matrixSizeAir = matrixSize;
    HashMapAiroute* map = malloc(sizeof(HashMapAiroute));
    if (matrixSize/2 == 1)
        matrixSizeAir = 1;
    map->bucketsAir = calloc(matrixSizeAir, sizeof(EntryAiroute*));
    return map;
}

char toggle_air_route(HashMapAiroute* map, Coord key, Coord value, int value2, int matrixSize, HashMapPosition* mapPosition) {
    int i=0;
    int matrixSizeAir = matrixSize;
    if (matrixSize/2 == 1)
        matrixSizeAir = 1;
    unsigned int index = hashing(key, matrixSizeAir);
    EntryAiroute* entry = map->bucketsAir[index];
    while (entry != NULL) {
        if (checkKey(entry->key, key)) {
            ValueAir *entryCopy = entry->value;
            ValueAir *temp = NULL;                                  //MANCA FUNZIONE PER CALCOLARE IL COSTO DELLA ROTTA AEREA (VALUE2)
            while (entryCopy != NULL) {
                if (checkKey(entryCopy->value1, value)) {
                    if (temp == NULL) {
                        entry->value = entryCopy->next;
                        free(entryCopy);
                        char *message = "OK";
                        return *message;
                    }
                    temp->next = entryCopy->next;
                    free(entryCopy);
                    char *message = "OK";
                    return *message;
                }
                temp = entryCopy;
                entryCopy = entryCopy->next;
                i++;
            }
            if (i<5 && getPair(mapPosition, key, matrixSize) != 0.5f && getPair(mapPosition, value, matrixSize) != 0.5f) {
                ValueAir *newEntry = malloc(sizeof(ValueAir));
                newEntry->value1 = value;
                newEntry->routeCost = value2;
                newEntry->next = entry->value;
                entry->value = newEntry;
                char *message = "OK";
                return *message;
            }
            char *message = "KO";
            return *message;
        }
        entry = entry->next;
    }
    if (getPair(mapPosition, key, matrixSize) == 0.5f || getPair(mapPosition, value, matrixSize) == 0.5f) {
        char *message = "KO";
        return *message;
    }
    entry = malloc(sizeof(EntryAiroute));
    entry->key = key;
    entry->value = malloc(sizeof(ValueAir));
    entry->value->value1 = value;
    entry->value->routeCost = value2;
    entry->value->next = NULL;
    entry->next = map->bucketsAir[index];
    map->bucketsAir[index] = entry;
    char *message = "OK";
    return *message;
}

void modifyValue(HashMapPosition* map, Coord key, int value, int matrixSize) {
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

float getPair(HashMapPosition* map, Coord key, int matrixSize) {
    if (key.x<0 || key.y<0 || key.x>=matrixSize || key.y>=matrixSize) {
        return 0.5f;
    }
    unsigned int index = hashing(key, matrixSize);
    Entry* entry = map->buckets[index];
    while (entry != NULL) {
        if (checkKey(entry->key, key)) {//RICONTROLLA QUI
            return entry->value;
        }
        entry = entry->next;
    }
    return 0.5f;
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
    int matrixSizeAir = matrixSize;
    if (matrixSize/2 == 1)
        matrixSizeAir = 1;
    for (int i = 0; i < matrixSizeAir; i++) {
        EntryAiroute* entry = map->bucketsAir[i];
        while (entry != NULL) {
            ValueAir *entryCopy = entry->value;
            while (entryCopy != NULL) {
                ValueAir *temp = entryCopy;
                entryCopy = entryCopy->next;
                free(temp);
            }
            EntryAiroute* next = entry->next;
            free(entry);
            entry = next;
        }
    }
    free(map->bucketsAir);
    free(map);
}

int distEsagoni(Coord a, Coord b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    if (dx * dy >= 0) {
        return abs(dx) + abs(dy);
    }
    if (abs(dx) > abs(dy)) {
        return abs(dx);
    }
    return abs(dy);
}

int change_cost(Coord key, int value, int ray, int matrixSize, HashMapPosition* mapPosition, HashMapAiroute* mapAiroute) {
    if (ray > 10 || ray < -10)
        return 1;
    if (getPair(mapPosition, key, matrixSize) == 0.5f) {
        return 1;
    }
    if (value > 10 || value < -10) {
        return 1;
    }


    return 0;
}

int main(int argc, const char *argv[]) {
    HashMapPosition* map = NULL;
    HashMapAiroute* mapAiroute = NULL;
    int x, y, precX, precY;
    while (1) {
        precX = x;
        precY = y;
        scanf("%d %d", &x, &y);
        if (x == -1 && y == -1) {
            break;
        }
        if (map != NULL) {
            deletePos(map, precX * precY);
            deleteAiroute(mapAiroute, precX * precY);
        }
        map = init(x * y, x);
        mapAiroute = create2(x * y);
        printf("%c\n", toggle_air_route(mapAiroute, (Coord){0, 1}, (Coord){5, 5}, 1, x * y, map));
        printf("%c\n", toggle_air_route(mapAiroute, (Coord){0, 1}, (Coord){1, 1}, 2, x * y, map));

    }
    deleteAiroute(mapAiroute, precX * precY);
    deletePos(map, precX * precY);
    return 0;
}
