#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    int x, y;
} Coord;

typedef struct {
    int x, y, z, w;
} DoubleCoord;

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

typedef struct EntryDouble {
    DoubleCoord key;
    int value;
    struct EntryDouble* next;
} EntryDouble;

typedef struct EntryAiroute {
    Coord key;
    ValueAir *value;
    struct EntryAiroute* next;
} EntryAiroute;

typedef struct {
    Entry** buckets;
} HashMapPosition;

typedef struct {
    EntryAiroute** bucketsAir;
} HashMapAiroute;

typedef struct {
    EntryDouble** bucketsDouble;
} HashMapCache;

typedef struct Node {
    Coord pos;
    int gCost;
    int hCost;
    int fCost;
    struct Node* prec;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
} PriorityQueue;

//Funzione hash
unsigned int hashing(Coord coord, int matrixSize) {
    unsigned int hash = coord.x * 31 + coord.y;
    return hash % matrixSize;
}

unsigned int hashingDouble(DoubleCoord coord, int matrixSize) {
    unsigned int hash = (coord.x * 31 + coord.y) * 31 + (coord.z * 31 + coord.w);
    return hash % matrixSize;
}

int checkKey(Coord a, Coord b) {
    return (a.x == b.x) && (a.y == b.y);
}

int checkKeyDouble(DoubleCoord a, DoubleCoord b) {
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
}

float getPair(HashMapPosition* map, Coord key, int matrixSize);

int col = 0, row = 0;

HashMapPosition* init(int matrixSize, int raw) {
    int j = 0, k = 0;
    col = raw;
    row = matrixSize / raw;
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

HashMapCache* createCache(int matrixSize) {
    HashMapCache* map = malloc(sizeof(HashMapCache));
    map->bucketsDouble = calloc(matrixSize, sizeof(EntryDouble*));
    return map;
}

int calculateRouteCost(HashMapAiroute* map, HashMapPosition* mapPosition, Coord key, int matrixSize) {
    float dcost = getPair(mapPosition, key, matrixSize);
    if (dcost == 0.5f) {
        return -1;
    }
    int matrixSizeAir = matrixSize;
    if (matrixSize/2 == 1)
        matrixSizeAir = 1;
    unsigned int index = hashing(key, matrixSizeAir);
    EntryAiroute* entry = map->bucketsAir[index];
    while (entry != NULL) {
        if (checkKey(entry->key, key)) {
            int totCost = (int)dcost;
            int count = 1;
            ValueAir* currentRoute = entry->value;
            while (currentRoute != NULL) {
                totCost += currentRoute->routeCost;
                count++;
                currentRoute = currentRoute->next;
            }
            return totCost / count;
        }
        entry = entry->next;
    }
    return (int)dcost;
}

int toggle_air_route(HashMapAiroute* map, Coord key, Coord value, int matrixSize, HashMapPosition* mapPosition) {
    int i=0;
    int matrixSizeAir = matrixSize;
    if (matrixSize/2 == 1)
        matrixSizeAir = 1;
    unsigned int index = hashing(key, matrixSizeAir);
    EntryAiroute* entry = map->bucketsAir[index];
    while (entry != NULL) {
        if (checkKey(entry->key, key)) {
            ValueAir *entryCopy = entry->value;
            ValueAir *temp = NULL;                                  //VA TESTATO FUNZIONAMENTO CALCULATEROUTECOST
            while (entryCopy != NULL) {
                if (checkKey(entryCopy->value1, value)) {
                    if (temp == NULL) {
                        entry->value = entryCopy->next;
                        free(entryCopy);
                        return 0;
                    }
                    temp->next = entryCopy->next;
                    free(entryCopy);
                    return 0;
                }
                temp = entryCopy;
                entryCopy = entryCopy->next;
                i++;
            }
            if (i<5 && getPair(mapPosition, key, matrixSize) != 0.5f && getPair(mapPosition, value, matrixSize) != 0.5f) {
                ValueAir *newEntry = malloc(sizeof(ValueAir));
                newEntry->value1 = value;
                newEntry->routeCost = calculateRouteCost(map, mapPosition, key, matrixSize);
                newEntry->next = entry->value;
                entry->value = newEntry;
                return 0;
            }
            return 1;
        }
        entry = entry->next;
    }
    if (getPair(mapPosition, key, matrixSize) == 0.5f || getPair(mapPosition, value, matrixSize) == 0.5f) {
        return 1;
    }
    entry = malloc(sizeof(EntryAiroute));
    entry->key = key;
    entry->value = malloc(sizeof(ValueAir));
    entry->value->value1 = value;
    entry->value->routeCost = calculateRouteCost(map, mapPosition, key, matrixSize);
    entry->value->next = NULL;
    entry->next = map->bucketsAir[index];
    map->bucketsAir[index] = entry;
    return 0;
}

void saveCache(HashMapCache* cache, DoubleCoord coord, int cost, int matrixSize) {
    unsigned int index = hashingDouble(coord, matrixSize);
    EntryDouble* entry = malloc(sizeof(EntryDouble));
    entry->key = coord;
    entry->value = cost;
    entry->next = cache->bucketsDouble[index];
    cache->bucketsDouble[index] = entry;
}

int getCache(HashMapCache* cache, DoubleCoord coord, int matrixSize) {
    unsigned int index = hashingDouble(coord, matrixSize);
    EntryDouble* entry = cache->bucketsDouble[index];
    while (entry != NULL) {
        if (checkKeyDouble(entry->key, coord)) {
            return entry->value;
        }
        entry = entry->next;
    }
    return -2;
}

void modifyValue(HashMapPosition* map, Coord key, int value, int matrixSize) {
    unsigned int index = hashing(key, matrixSize);
    Entry* entry = map->buckets[index];
    while (entry != NULL) {
        if (checkKey(entry->key, key)) {
            entry->value += value;
            if (entry->value < 0) {
                entry->value = 0;
            }
            if (entry->value > 100) {
                entry->value = 100;
            }
            return;
        }
        entry = entry->next;
    }
}

float getPair(HashMapPosition* map, Coord key, int matrixSize) {
    if (key.x<0 || key.y<0 || key.x>=col || key.y>=row) {
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

void deleteCache(HashMapCache* cache, int matrixSize) {
    for (int i = 0; i < matrixSize; i++) {
        EntryDouble* entry = cache->bucketsDouble[i];
        while (entry != NULL) {
            EntryDouble* next = entry->next;
            free(entry);
            entry = next;
        }
    }
    free(cache->bucketsDouble);
    free(cache);
}

void freeQueue(PriorityQueue* queue) {
    while (queue->head != NULL) {
        Node* node = queue->head;
        queue->head = queue->head->next;
        free(node);
    }
    free(queue);
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

void modifyAirRoutesCost(HashMapAiroute* mapAiroute, Coord key, int costChange, int matrixSize) {
    int matrixSizeAir = matrixSize;
    if (matrixSize/2 == 1)
        matrixSizeAir = 1;
    unsigned int index = hashing(key, matrixSizeAir);
    EntryAiroute* entry = mapAiroute->bucketsAir[index];
    while (entry != NULL) {
        if (checkKey(entry->key, key)) {
            ValueAir* currentRoute = entry->value;
            while (currentRoute != NULL) {
                currentRoute->routeCost += costChange;
                if (currentRoute->routeCost < 0) {
                    currentRoute->routeCost = 0;
                }
                currentRoute = currentRoute->next;
            }
            return;
        }
        entry = entry->next;
    }
}

int change_cost(Coord key, int value, int ray, int matrixSize, HashMapPosition* mapPosition, HashMapAiroute* mapAiroute) {
    int dcost = 0;
    if (ray > 10 || ray < -10)
        return 1;
    if (getPair(mapPosition, key, matrixSize) == 0.5f) {
        return 1;
    }
    if (value > 10 || value < -10) {
        return 1;
    }
    for (int i = key.x-ray; i < ray+key.x; i++) {
        for (int j = key.y-ray; j < ray+key.y; j++) {  //VA ANCORA TESTATO PER CAPIRE SE VA (A LOGICA SI MA CON LA LOGICA VAI CONTRO IL MURO)
            if (i < 0 || j < 0) continue;
            int distance = distEsagoni(key, (Coord){i,j});
            if (distance >= ray) continue;
            dcost = (ray - distance)/ray;
            if (dcost < 0) {
                dcost = 0;
            }
            dcost = value * dcost;
            if (getPair(mapPosition, (Coord){i,j}, matrixSize) != 0.5f) {
                modifyValue(mapPosition, (Coord){i,j}, dcost, matrixSize);
                modifyAirRoutesCost(mapAiroute, (Coord){i,j}, dcost, matrixSize);
            }
        }
    }
    return 0;
}

int getHexNear(Coord pos, HashMapPosition* mapPosition, HashMapAiroute* mapAiroute, int matrixSize, Coord* hexNearVect, int* costs) {
    int count = 0;
    float posCost = getPair(mapPosition, pos, matrixSize);
    if (posCost == 0.5f) return 0;
    int dx[] = {-1, 1, 0, 0, -1, 1};
    int dy[] = {0, 0, -1, 1, 1, -1};
    for (int i = 0; i < 6; i++) {
        Coord hexNear = {pos.x + dx[i], pos.y + dy[i]};
        float neighborCost = getPair(mapPosition, hexNear, matrixSize);
        if (neighborCost != 0.5f && neighborCost > 0) {
            hexNearVect[count] = hexNear;
            costs[count] = (int)posCost;
            count++;
        }
    }
    int matrixSizeAir = matrixSize;
    if (matrixSize/2 == 1) matrixSizeAir = 1;
    unsigned int index = hashing(pos, matrixSizeAir);
    EntryAiroute* entry = mapAiroute->bucketsAir[index];
    while (entry != NULL) {
        if (checkKey(entry->key, pos)) {
            ValueAir* currentRoute = entry->value;
            while (currentRoute != NULL && count < 11) {
                if (getPair(mapPosition, currentRoute->value1, matrixSize) != 0.5f) {
                    hexNearVect[count] = currentRoute->value1;
                    costs[count] = currentRoute->routeCost;
                    count++;
                }
                currentRoute = currentRoute->next;
            }
            break;
        }
        entry = entry->next;
    }
    return count;
}

void enqueue(PriorityQueue* queue, Node* node) {
    if (queue->head == NULL || node->fCost < queue->head->fCost) {
        node->next = queue->head;
        queue->head = node;
    } else {
        Node* current = queue->head;
        while (current->next != NULL && current->next->fCost <= node->fCost) {
            current = current->next;
        }
        node->next = current->next;
        current->next = node;
    }
}

Node* findInList(Node* list, Coord pos) {
    while (list != NULL) {
        if (checkKey(list->pos, pos)) {
            return list;
        }
        list = list->next;
    }
    return NULL;
}

int astar(Coord start, Coord dest, HashMapPosition* mapPosition, HashMapAiroute* mapAiroute, int matrixSize) {
    if (getPair(mapPosition, start, matrixSize) == 0.5f ||
        getPair(mapPosition, dest, matrixSize) == 0.5f) {
        return -1;
    }
    if (checkKey(start, dest)) {
        return 0;
    }

    PriorityQueue* queue = malloc(sizeof(PriorityQueue));
    queue->head = NULL;
    Node* visitedNodes = NULL;
    Node* startNode = malloc(sizeof(Node));
    startNode->pos = start;
    startNode->gCost = 0;
    startNode->hCost = distEsagoni(start, dest);
    startNode->fCost = startNode->gCost + startNode->hCost;
    startNode->prec = NULL;
    startNode->next = NULL;

    enqueue(queue, startNode);

    while (queue->head != NULL) {
        Node* current = queue->head;
        queue->head = queue->head->next;
        if (checkKey(current->pos, dest)) {
            int totCost = current->gCost;
            freeQueue(queue);
            while (visitedNodes != NULL) {
                Node* temp = visitedNodes;
                visitedNodes = visitedNodes->next;
                free(temp);
            }
            free(current);
            return totCost;
        }
        current->next = visitedNodes;
        visitedNodes = current;
        Coord hexNearVect[11];
        int costs[11];
        int hexNearCount = getHexNear(current->pos, mapPosition, mapAiroute,matrixSize, hexNearVect, costs);
        for (int i = 0; i < hexNearCount; i++) {
            Coord hexPos = hexNearVect[i];
            int moveCost = costs[i];
            if (moveCost <= 0) continue;
            if (findInList(visitedNodes, hexPos) != NULL) continue;
            int tentativeGCost = current->gCost + moveCost;

            Node* neighborNode = malloc(sizeof(Node));
            neighborNode->pos = hexPos;
            neighborNode->gCost = tentativeGCost;
            neighborNode->hCost = distEsagoni(hexPos, dest);
            neighborNode->fCost = neighborNode->gCost + neighborNode->hCost;
            neighborNode->prec = current;
            neighborNode->next = NULL;
            enqueue(queue, neighborNode);
        }
    }
    freeQueue(queue);
    while (visitedNodes != NULL) {
        Node* temp = visitedNodes;
        visitedNodes = visitedNodes->next;
        free(temp);
    }
    return -1;
}

int travel_cost(Coord start, Coord dest, HashMapPosition* mapPosition, HashMapAiroute* mapAiroute, HashMapCache* cache, int matrixSize) {
    DoubleCoord coord = {start.x, start.y, dest.x, dest.y};
    int c = getCache(cache, coord, matrixSize);
    if (c != -2) {
        return c;
    }
    int a = astar(start, dest, mapPosition, mapAiroute, matrixSize);
    if (a >= 0) {
        saveCache(cache, coord, a, matrixSize);
    }
    return a;
}


int main(int argc, const char *argv[]) { //verifica bounds in getpair
    char command[255];
    HashMapPosition* map = NULL;
    HashMapAiroute* mapAiroute = NULL;
    HashMapCache* mapCache = NULL;
    int precX = 0, precY = 0, x = 0, y = 0, v = 0, ray = 0, initial = 0;
    while (!feof(stdin)) {
        if (scanf("%s", command)!=1) {
            deletePos(map, precX * precY);
            deleteAiroute(mapAiroute, precX * precY);
            deleteCache(mapCache, precX * precY);
            return 0;
        }
        if (strcmp(command, "init") == 0) {
            if (initial == 1) {
                deletePos(map, precX * precY);
                deleteAiroute(mapAiroute, precX * precY);
                deleteCache(mapCache, precX * precY);
                initial = 0;
            }
            if(scanf("%d", &precX) != 1) {
                continue;
            }
            if (scanf("%d", &precY) != 1) {
                continue;
            }
            printf("OK\n");
            initial = 1;
            map = init(precY*precX, precX);
            mapAiroute = create2(precX * precY);
            mapCache = createCache(precX * precY);
        }
        else if (strcmp(command, "change_cost") == 0) {
            if(scanf("%d", &x) != 1) {
                continue;
            }
            if (scanf("%d", &y) != 1) {
                continue;
            }
            if(scanf("%d", &v) != 1) {
                continue;
            }
            if (scanf("%d", &ray) != 1) {
                continue;
            }
            if (change_cost((Coord){x,y}, v, ray, precX * precY, map, mapAiroute) == 0) {
                printf("OK\n");
                deleteCache(mapCache, precX * precY);
                mapCache = createCache(precX * precY);
            }
            else {
                printf("KO\n");
            }
        }
        else if (strcmp(command, "toggle_air_route") == 0) {
            if(scanf("%d", &x) != 1) {
                continue;
            }
            if (scanf("%d", &y) != 1) {
                continue;
            }
            if (scanf("%d", &v) != 1) {
                continue;
            }
            if (scanf("%d", &ray) != 1) {
                continue;
            }
            if (toggle_air_route(mapAiroute, (Coord){x,y}, (Coord){v,ray}, precX * precY, map) == 0) {
                printf("OK\n");
                deleteCache(mapCache, precX * precY);
                mapCache = createCache(precX * precY);
            }
            else {
                printf("KO\n");
            }
        }
        else if (strcmp(command, "travel_cost") == 0) {
            if (scanf("%d", &x) != 1) {
                continue;
            }
            if (scanf("%d", &y) != 1) {
                continue;
            }
            if (scanf("%d", &v) != 1) {
                continue;
            }
            if (scanf("%d", &ray) != 1) {
                continue;
            }
            int res = travel_cost((Coord){x, y}, (Coord){v, ray}, map, mapAiroute, mapCache, precX * precY);
            printf("%d\n", res);
        }
    }
    deletePos(map, precX * precY);
    deleteAiroute(mapAiroute, precX * precY);
    deleteCache(mapCache, precX * precY);
    return 0;
}
