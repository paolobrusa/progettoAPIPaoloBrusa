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

typedef struct {
    int* value;
    Coord key;
} HashMapPosition;

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

float getPair(HashMapPosition* map, Coord key);

HashMapPosition* init(int raw, int columns) {
    HashMapPosition* map = malloc(sizeof(HashMapPosition));
    map->key = (Coord){raw, columns};
    map->value = malloc(columns * raw * sizeof(int));
    int matrixSize = columns * raw;
    for (int i = 0; i < matrixSize; i++) {
        map->value[i] = 1;
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
    float dcost = getPair(mapPosition, key);
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
            //printf("totcost %d\n", totCost);
            return totCost / count;
        }
        entry = entry->next;
    }
    //printf("dcost %f\n", dcost);
    return (int)dcost;
}

int toggle_air_route(HashMapAiroute* map, Coord key, Coord value, int matrixSize, HashMapPosition* mapPosition) {
    if (key.x < 0 || key.y < 0 || key.x >= mapPosition->key.x || key.y >= mapPosition->key.y ||
       value.x < 0 || value.y < 0 || value.x >= mapPosition->key.x || value.y >= mapPosition->key.y) {
        return 1;
       }
    if (getPair(mapPosition, key) == 0.5f || getPair(mapPosition, value) == 0.5f) {
        return 1;
    }
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
            if (i<5) {
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

void modifyValue(HashMapPosition* map, Coord key, int value) {
    if (key.x < 0 || key.y < 0 || key.x >= map->key.x || key.y >= map->key.y) {
        return;
    }

    int index = key.x * map->key.y + key.y;
    map->value[index] += value;

    if (map->value[index] < 0) {
        map->value[index] = 0;
    }
    if (map->value[index] > 100) {
        map->value[index] = 100;
    }
}

float getPair(HashMapPosition* map, Coord key) {
    if (key.x < 0 || key.y < 0 || key.x >= map->key.x || key.y >= map->key.y) {
        return 0.5f;
    }
    return map->value[key.x * map->key.y + key.y];
}

void deletePos(HashMapPosition* map) {
    free(map->value);
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

typedef struct {
    int q;
    int r;
    int s;
} Cube;

Cube coordCube(Coord a) {
    int q = a.x - (a.y - (a.y & 1)) / 2;
    int r = a.y;
    int s = -q - r;
    return (Cube){q, r, s};
}
// Function to create a new Cube
Cube createCube(int q, int r, int s) {
    Cube cube;
    cube.q = q;
    cube.r = r;
    cube.s = s;
    return cube;
}

Cube newCube(Cube a, Cube b) {
    return createCube(a.q - b.q, a.r - b.r, a.s - b.s);
}

int distEsagoni(Coord a, Coord b) {
    Cube cube = newCube(coordCube(a), coordCube(b));
    int aq = abs(cube.q), ar = abs(cube.r), as = abs(cube.s);
    int max = (aq > ar) ? ((aq > as) ? aq : as) : ((ar > as) ? ar : as);
    return max;
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
    //printf("change_cost: key=(%d,%d), value=%d, ray=%d\n", key.x, key.y, value, ray);
    int dcost = 0;
    double c = 0;
    if (ray <= 0)
        return 1;
    if (getPair(mapPosition, key) == 0.5f) {
        return 1;
    }
    if (value > 10 || value < -10) {
        return 1;
    }
    for (int i = key.x-ray; i <= key.x+ray; i++) {
        for (int j = key.y-ray; j <= key.y+ray; j++) {  //VA ANCORA TESTATO PER CAPIRE SE VA (A LOGICA SI MA CON LA LOGICA VAI CONTRO IL MURO)
            if (i < 0 || j < 0 || i >= mapPosition->key.x || j >= mapPosition->key.y) continue;
            int distance = distEsagoni(key, (Coord){i,j});
            //printf("esagono (%d,%d), distance=%d\n", i, j, distance);
            if (distance >= ray) continue;
            //printf("esagono (%d,%d), distance=%d\n", i, j, distance);
            c = (double)(ray - distance)/ray;
            if (c < 0) {
                c = 0;
            }
            c = value * c;
            dcost = c>=0 ? (int)c : (c!=(int)c ? (int)c-1 : (int)c);
            //printf("modificando (%d,%d) con dcost=%d\n", i, j, dcost);
            if (getPair(mapPosition, (Coord){i,j}) != 0.5f) {
                modifyValue(mapPosition, (Coord){i,j}, dcost);
                modifyAirRoutesCost(mapAiroute, (Coord){i,j}, dcost, matrixSize);
            }
        }
    }
    return 0;
}

typedef struct Tp{
    Coord pos;
    Coord dest;
    int cost;
    int routeCost;
} Tp;

int getHexNear(Coord pos, HashMapPosition* mapPosition, Coord* hexNearVect, int* costs, Coord dest, Tp* tp, int tpcount) {
    //printf("DEBUG getHexNear chiamata con pos=(%d,%d)\n", pos.x, pos.y);
    int count = 0;
    float pc = getPair(mapPosition, pos);
    if (pc == 0.5f || pc==0) return 0;
    int posCost = (int)pc;

    static Coord offsetEven[6] = {{+1,0}, {0,-1}, {-1, -1}, {-1,0}, {-1,+1}, {0, +1}};
    static Coord offsetOdd[6] = {{+1,0}, {+1,-1}, {0, -1}, {-1,0}, {0,+1}, {+1, +1}};

    const Coord* offset = (pos.y % 2 == 0) ? offsetEven : offsetOdd;

    for (int i = 0; i < 6; i++) {
        Coord hexNear = {pos.x + offset[i].x, pos.y + offset[i].y};
        float neighborCost = getPair(mapPosition, hexNear);
        if (neighborCost != 0.5f && (neighborCost > 0 || (hexNear.x == dest.x && hexNear.y == dest.y) )) {
            hexNearVect[count] = hexNear;
            costs[count] = posCost;
            count++;
        }
    }

    if (tp != NULL && tpcount > 0) {
        for (int t = 0; t < tpcount; t++) {
            //printf("DEBUG: Teletrasporto %d: da (%d,%d) a (%d,%d)\n",t, tp[t].pos.x, tp[t].pos.y, tp[t].dest.x, tp[t].dest.y);
            if (checkKey(pos, tp[t].pos)) {
                //printf("DEBUG: TELETRASPORTO ATTIVATO Da (%d,%d) a (%d,%d) con costo %d\n",pos.x, pos.y, tp[t].dest.x, tp[t].dest.y, tp[t].routeCost);
                hexNearVect[count] = tp[t].dest;
                costs[count] = tp[t].routeCost;
                count++;
            }
        }
    }
    return count;
}

typedef struct {
    Node* buckets[100000];
    int minCost;
    int maxCost;
    int size;
} BucketQueue;

BucketQueue* createBucketQueue() {
    BucketQueue* queue = malloc(sizeof(BucketQueue));
    memset(queue->buckets, 0, sizeof(queue->buckets));
    queue->minCost = 100000;
    queue->maxCost = -1;
    queue->size = 0;
    return queue;
}

void bucketPush(BucketQueue* queue, Node* node) {
    int cost = node->fCost;
    //printf("DEBUG bucketPush: aggiungendo (%d,%d) con fCost=%d\n", node->pos.x, node->pos.y, cost);
    if (cost >= 100000) cost = 100000 - 1;
    node->next = queue->buckets[cost];
    queue->buckets[cost] = node;
    if (cost < queue->minCost) queue->minCost = cost;
    if (cost > queue->maxCost) queue->maxCost = cost;
    queue->size++;
}

Node* bucketPop(BucketQueue* queue) {
    if (queue->size == 0) return NULL;
    while (queue->minCost <= queue->maxCost && queue->buckets[queue->minCost] == NULL) {
        queue->minCost++;
    }
    if (queue->minCost > queue->maxCost) {
        queue->size = 0;
        return NULL;
    }
    Node* node = queue->buckets[queue->minCost];
    //printf("DEBUG bucketPop: estraendo (%d,%d) con fCost=%d\n", node->pos.x, node->pos.y, node->fCost);
    queue->buckets[queue->minCost] = node->next;
    node->next = NULL;
    queue->size--;
    return node;
}

void deleteBucketQueue(BucketQueue* queue) {
    for (int i = 0; i < 100000; i++) {
        Node* current = queue->buckets[i];
        while (current != NULL) {
            Node* next = current->next;
            free(current);
            current = next;
        }
    }
    free(queue);
}


typedef struct HashSetEntry {
    Coord key;
    struct HashSetEntry* next;
} HashSetEntry;

typedef struct {
    HashSetEntry** buckets;
    int size;
} HashSet;

HashSet* createHashSet(int size) {
    HashSet* set = malloc(sizeof(HashSet));
    set->size = size;
    set->buckets = calloc(size, sizeof(HashSetEntry*));
    return set;
}

int hashSetContains(HashSet* set, Coord key) {
    unsigned int index = hashing(key, set->size);
    HashSetEntry* entry = set->buckets[index];
    while (entry != NULL) {
        if (checkKey(entry->key, key)) {
            return 0;
        }
        entry = entry->next;
    }
    return 1;
}

void deleteHashSet(HashSet* set) {
    for (int i = 0; i < set->size; i++) {
        HashSetEntry* entry = set->buckets[i];
        while (entry != NULL) {
            HashSetEntry* next = entry->next;
            free(entry);
            entry = next;
        }
    }
    free(set->buckets);
    free(set);
}

static HashSetEntry* pool = NULL;
static int poolIndex = 0;
static int poolCapacity = 0;

void createPool(int matrixSize) {
    if (pool) {
        free(pool);
    }
    poolCapacity = matrixSize*5;
    pool = malloc(poolCapacity * sizeof(HashSetEntry));
    poolIndex = 0;
}

void resetPool() {
    poolIndex = 0;
}

void deletePool() {
    if (pool) {
        free(pool);
        pool = NULL;
    }
}

void hashSetAdd(HashSet* set, Coord key) {
    if (hashSetContains(set, key) == 0) {
        return;
    }
    unsigned int index = hashing(key, set->size);
    HashSetEntry* entry;
    if (poolIndex < poolCapacity) {
        entry = &pool[poolIndex++];
    } else {
        poolIndex = 0;
        for (int i = 0; i < set->size; i++) {
            set->buckets[i] = NULL;
        }
        entry = &pool[poolIndex++];
    }
    entry->key = key;
    entry->next = set->buckets[index];
    set->buckets[index] = entry;
}

int heuristicFun(Coord start, Coord dest, Tp* tp, int tpcount) {
    int distance = distEsagoni(start, dest);

    if (tp != NULL && tpcount > 0) {
        int bestCost = distance;

        // Controlla tutti i teleport possibili
        for (int t = 0; t < tpcount; t++) {
            int distToTp = distEsagoni(start, tp[t].pos);
            int distFromTpToDest = distEsagoni(tp[t].dest, dest);

            // Costo totale: raggiungere il teleport + usare il teleport + raggiungere destinazione
            int totalCost = distToTp + 1 + distFromTpToDest; // +1 per il costo del teleport

            if (totalCost < bestCost) {
                bestCost = totalCost;
            }
        }

        // Controlla anche catene di teleport (max 2 hop)
        for (int t1 = 0; t1 < tpcount; t1++) {
            int distToTp1 = distEsagoni(start, tp[t1].pos);

            // Se il primo teleport è ragionevolmente vicino
            if (distToTp1 <= distance / 2) {
                for (int t2 = 0; t2 < tpcount; t2++) {
                    if (t1 != t2) {
                        int distBetweenTps = distEsagoni(tp[t1].dest, tp[t2].pos);
                        int distToFinalDest = distEsagoni(tp[t2].dest, dest);

                        // Costo catena: reach tp1 + use tp1 + reach tp2 + use tp2 + reach dest
                        int chainCost = distToTp1 + 1 + distBetweenTps + 1 + distToFinalDest;

                        if (chainCost < bestCost) {
                            bestCost = chainCost;
                        }
                    }
                }
            }
        }

        distance = bestCost;
    }

    return distance;
}

int astar(Coord start, Coord dest, HashMapPosition* mapPosition, HashMapAiroute* mapAiroute, int matrixSize, HashSet* visitedNodes, Tp* tp, int tpcount) {
    //printf("DEBUG A*: Inizio da (%d,%d) a (%d,%d)\n", start.x, start.y, dest.x, dest.y);
    if (getPair(mapPosition, start) == 0.5f ||
        getPair(mapPosition, dest) == 0.5f) {
        return -1;
    }
    if (checkKey(start, dest)) {
        return 0;
    }
    BucketQueue* queue = createBucketQueue();
    HashMapCache* gCosts = createCache(matrixSize/4);
    Node* startNode = malloc(sizeof(Node));
    startNode->pos = start;
    startNode->gCost = 0;
    startNode->hCost = heuristicFun(start, dest, tp, tpcount);
    startNode->fCost = startNode->gCost + startNode->hCost;
    startNode->prec = NULL;
    startNode->next = NULL;

    bucketPush(queue, startNode);
    DoubleCoord startCoord = {start.x, start.y, 0, 0};
    saveCache(gCosts, startCoord, 0, matrixSize/4);

    while (queue->size > 0) {
        Node* current = bucketPop(queue);
        if (current == NULL) break;
        //printf("DEBUG A*: processando nodo (%d,%d) con gCost=%d\n", current->pos.x, current->pos.y, current->gCost);
        if (checkKey(current->pos, dest)) {
            int totCost = current->gCost;
            free(current);
            deleteBucketQueue(queue);
            deleteCache(gCosts, matrixSize/4);
            resetPool();
            memset(visitedNodes->buckets, 0, visitedNodes->size * sizeof(HashSetEntry*));
            return totCost;
        }
        DoubleCoord currentCoord = {current->pos.x, current->pos.y, 0, 0};
        int cachedGCost = getCache(gCosts, currentCoord, matrixSize/4);
        if (cachedGCost != -2 && cachedGCost < current->gCost) {
            free(current);
            continue;
        }
        hashSetAdd(visitedNodes, current->pos);
        Coord hexNearVect[11];
        int costs[11];
        int hexNearCount = getHexNear(current->pos, mapPosition, hexNearVect, costs, dest, tp, tpcount);
        for (int i = 0; i < hexNearCount; i++) {
            Coord hexPos = hexNearVect[i];
            int cost = costs[i];
            if (cost <= 0) continue;
            if (hashSetContains(visitedNodes, hexPos) == 0) continue;
            int tentativeGCost = current->gCost + cost;
            DoubleCoord neighborCoord = {hexPos.x, hexPos.y, 0, 0};
            int existingGCost = getCache(gCosts, neighborCoord, matrixSize/4);
            if (existingGCost != -2 && existingGCost <= tentativeGCost) {
                continue;
            }
            saveCache(gCosts, neighborCoord, tentativeGCost, matrixSize/4);
            Node* near = malloc(sizeof(Node));
            near->pos = hexPos;
            near->gCost = tentativeGCost;
            near->hCost = heuristicFun(hexPos, dest, tp, tpcount);
            near->fCost = tentativeGCost + near->hCost;
            near->prec = current;
            near->next = NULL;
            bucketPush(queue, near);
        }
        free(current);
    }
    deleteBucketQueue(queue);
    deleteCache(gCosts, matrixSize/4);
    resetPool();
    memset(visitedNodes->buckets, 0, visitedNodes->size * sizeof(HashSetEntry*));
    return -1;
}

Tp* airRouteTp(Coord dest, HashMapAiroute* mapAiroute, HashMapPosition* mapPosition, int matrixSize, int* tpcount) {
    *tpcount = 0;
    Tp* tp = malloc(100 * sizeof(Tp));
    int matrixSizeAir = matrixSize;
    if (matrixSize/2 == 1) matrixSizeAir = 1;

    int maxDistance = 1000;

    for (int i = 0; i < matrixSizeAir; i++) {
        EntryAiroute* entry = mapAiroute->bucketsAir[i];
        while (entry != NULL) {
            ValueAir* currentRoute = entry->value;
            while (currentRoute != NULL) {
                int distToDest = distEsagoni(currentRoute->value1, dest);

                if (distToDest <= maxDistance) {
                    int directDistance = distEsagoni(entry->key, dest);

                    if (currentRoute->routeCost + distToDest <= directDistance + 10) {
                        tp[*tpcount].pos = entry->key;
                        tp[*tpcount].dest = currentRoute->value1;
                        tp[*tpcount].cost = (int)getPair(mapPosition, entry->key);
                        tp[*tpcount].routeCost = currentRoute->routeCost;
                        (*tpcount)++;
                       // printf("DEBUG: Teletrasporto candidato da (%d,%d) a (%d,%d) costo=%d, dist_finale=%d\n",entry->key.x, entry->key.y, currentRoute->value1.x, currentRoute->value1.y, currentRoute->routeCost, distToDest);
                        if (*tpcount >= 100) break;
                    }
                }
                currentRoute = currentRoute->next;
            }
            if (*tpcount >= 100) break;
            entry = entry->next;
        }
        if (*tpcount >= 100) break;
    }
    //printf("DEBUG: Trovati %d teletrasporti candidati\n", *tpcount);
    return tp;
}


int travel_cost(Coord start, Coord dest, HashMapPosition* mapPosition, HashMapAiroute* mapAiroute, HashMapCache* cache, int matrixSize, HashSet* visitedNodes, HashSet* dup) {
    DoubleCoord coord = {start.x, start.y, dest.x, dest.y};
    int c = getCache(cache, coord, matrixSize);
    if (c != -2) {
        return c;
    }
    int tpcount = 0;
    Tp* tp = NULL;
    tp = airRouteTp(dest, mapAiroute ,mapPosition, matrixSize, &tpcount);
    int a = astar(start, dest, mapPosition, mapAiroute, matrixSize, visitedNodes, tp, tpcount);
    if (a != -1) {
        saveCache(cache, coord, a, matrixSize);
    }
    if (tp != NULL) {
        free(tp);
    }
    return a;
}


int main(int argc, const char *argv[]) {
    char command[255];
    HashMapPosition* map = NULL;
    HashMapAiroute* mapAiroute = NULL;
    HashMapCache* mapCache = NULL;
    HashSet* visitedNodes = NULL;
    HashSet* dup = NULL;
    int precX = 0, precY = 0, x = 0, y = 0, v = 0, ray = 0, initial = 0;
    while (!feof(stdin)) {
        if (scanf("%s", command)!=1) {
            deletePos(map);
            deleteAiroute(mapAiroute, precX * precY);
            deleteCache(mapCache, precX * precY);
            deleteHashSet(visitedNodes);
            deletePool();
            return 0;
        }
        if (strcmp(command, "init") == 0) {
            if (initial == 1) {
                deletePos(map);
                deleteAiroute(mapAiroute, precX * precY);
                deleteCache(mapCache, precX * precY);
                deleteHashSet(visitedNodes);
                deletePool();
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
            map = init(precX, precY);
            mapAiroute = create2(precX * precY);
            mapCache = createCache(precX * precY);
            visitedNodes = createHashSet(precX * precY);
            dup = createHashSet(precX * precY);
            createPool(precX * precY);
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
            if (initial != 1 || map == NULL || mapAiroute == NULL || mapCache == NULL) {
                printf("KO\n");
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
            if (initial != 1 || map == NULL || mapAiroute == NULL || mapCache == NULL) {
                printf("KO\n");
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
            if (initial != 1 || map == NULL || mapAiroute == NULL || mapCache == NULL) {
                printf("-1\n");
                continue;
            }
            int res = travel_cost((Coord){x, y}, (Coord){v, ray}, map, mapAiroute, mapCache, precX * precY, visitedNodes, dup);
            printf("%d\n", res);
        }
    }
    deletePos(map);
    deleteAiroute(mapAiroute, precX * precY);
    deleteCache(mapCache, precX * precY);
    deleteHashSet(visitedNodes);
    deleteHashSet(dup);
    deletePool();
    return 0;
}