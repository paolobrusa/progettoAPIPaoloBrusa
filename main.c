#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

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

typedef struct {
    int v;
    int p;
} edge;

typedef struct {
    edge **edges;
    int len;
    int size;
    int dist;
    int prev;
    int visited;
} Vert;

typedef struct {
    int *data;
    int *prio;
    int *index;
    int len;
    int size;
} Heap;

Heap *createHeap(int n) {
    Heap *h = calloc(1, sizeof(Heap));
    h->data = calloc(n + 1, sizeof(int));
    h->prio = calloc(n + 1, sizeof(int));
    h->index = calloc(n, sizeof(int));
    h->size = n;
    return h;
}

void push(Heap *h, int v, int p) {
    int i = h->index[v] == 0 ? ++h->len : h->index[v];
    int j = i >> 1;

    while (i > 1 && h->prio[j] > p) {
        h->data[i] = h->data[j];
        h->prio[i] = h->prio[j];
        h->index[h->data[i]] = i;
        i = j;
        j = j >> 1;
    }
    h->data[i] = v;
    h->prio[i] = p;
    h->index[v] = i;
}

static int minHeap(Heap *h, int i, int j, int k) {
    int m = i;
    if (j <= h->len && h->prio[j] < h->prio[m])
        m = j;
    if (k <= h->len && h->prio[k] < h->prio[m])
        m = k;
    return m;
}

int pop(Heap *h) {
    int v = h->data[1];
    int i = 1;

    while (1) {
        int left = i << 1;
        int right = left + 1;
        int j = minHeap(h, h->len, left, right);
        if (j == h->len) break;

        h->data[i] = h->data[j];
        h->prio[i] = h->prio[j];
        h->index[h->data[i]] = i;
        i = j;
    }

    h->data[i] = h->data[h->len];
    h->prio[i] = h->prio[h->len];
    h->index[h->data[i]] = i;
    h->len--;
    h->index[v] = 0;
    return v;
}

void destroy(Heap *h) {
    free(h->data);
    free(h->prio);
    free(h->index);
    free(h);
}

static int convInd(Coord pos, int cols) {
    return pos.x * cols + pos.y;
}

static Coord convCoord(int index, int cols) {
    return (Coord){index / cols, index % cols};
}

static int hexNear(Coord pos, int neighbors[6], int rows, int cols) {
    static const int offsetEven[6][2] = {{1,0}, {0,-1}, {-1,-1}, {-1,0}, {-1,1}, {0,1}};
    static const int offsetOdd[6][2] = {{1,0}, {1,-1}, {0,-1}, {-1,0}, {0,1}, {1,1}};

    const int (*offset)[2] = (pos.y & 1) ? offsetOdd : offsetEven;
    int count = 0;

    for (int i = 0; i < 6; i++) {
        int nx = pos.x + offset[i][0];
        int ny = pos.y + offset[i][1];

        if (nx >= 0 && nx < rows && ny >= 0 && ny < cols) {
            neighbors[count++] = nx * cols + ny;
        }
    }
    return count;
}

ValueAir* getAirRoutes(HashMapAiroute* mapAiroute, Coord pos, int matrixSize) {
    int matrixSizeAir = matrixSize;
    if (matrixSize/2 == 1)
        matrixSizeAir = 1;

    unsigned int index = hashing(pos, matrixSizeAir);
    EntryAiroute* entry = mapAiroute->bucketsAir[index];

    while (entry != NULL) {
        if (checkKey(entry->key, pos)) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

int travel_cost(Coord start, Coord end, HashMapPosition* mapPosition,HashMapAiroute* mapAiroute, HashMapCache* mapCache, int matrixSize) {
    if (start.x < 0 || start.x >= mapPosition->key.x || start.y < 0 || start.y >= mapPosition->key.y ||
        end.x < 0 || end.x >= mapPosition->key.x || end.y < 0 || end.y >= mapPosition->key.y) {
        return -1;
    }

    if (start.x == end.x && start.y == end.y) {
        return 0;
    }

    DoubleCoord cacheKey = {start.x, start.y, end.x, end.y};
    int cachedResult = getCache(mapCache, cacheKey, matrixSize);
    if (cachedResult != -2) {
        return cachedResult;
    }

    int rows = mapPosition->key.x;
    int cols = mapPosition->key.y;
    int totalNodes = rows * cols;

    int startIndex = convInd(start, cols);
    int endIndex = convInd(end, cols);

    float startCostFloat = getPair(mapPosition, start);
    float endCostFloat = getPair(mapPosition, end);
    if (startCostFloat == 0.5f || endCostFloat == 0.5f) {
        return -1;
    }
    if (startCostFloat == 0.0f) {
        return -1;
    }
    Vert *vert = calloc(totalNodes, sizeof(Vert));

    for (int i = 0; i < totalNodes; i++) {
        vert[i].dist = INT_MAX;
        vert[i].visited = 0;
        vert[i].prev = -1;
    }
    vert[startIndex].dist = 0;
    Heap *heap = createHeap(totalNodes);
    push(heap, startIndex, 0);

    int result = -1;

    while (heap->len > 0) {
        int index = pop(heap);
        // if (index == endIndex) {
        //     result = vertices[index].dist;
        //     break;
        // }
        if (vert[index].visited) continue;
        vert[index].visited = 1;
        Coord currentPos = convCoord(index, cols);
        if (currentPos.x == end.x && currentPos.y == end.y) {
            result = vert[index].dist;
            //printf("sono qui\n");
            break;
        }
        float costExit = getPair(mapPosition, currentPos);
        if (costExit == 0.5f) continue;
        if (costExit == 0.0f) continue;

        int currentExitCost = (int)costExit;

        int hexvec[6];
        int hexnear = hexNear(currentPos, hexvec, rows, cols);

        for (int i = 0; i < hexnear; i++) {
            int hex = hexvec[i];
            if (vert[hex].visited) continue;
            Coord neighborPos = convCoord(hex, cols);
            float neighborCostFloat = getPair(mapPosition, neighborPos);
            if (neighborCostFloat == 0.5f) continue;
            int newDist = vert[index].dist + currentExitCost;
            if (newDist < vert[hex].dist) {
                vert[hex].dist = newDist;
                vert[hex].prev = index;
                push(heap, hex, newDist);
            }
        }

        ValueAir* airRoutes = getAirRoutes(mapAiroute, currentPos, matrixSize);
        while (airRoutes != NULL) {
            Coord airDest = airRoutes->value1;
            int dest = convInd(airDest, cols);

            if (!vert[dest].visited) {
                float destCostFloat = getPair(mapPosition, airDest);
                if (destCostFloat != 0.5f) {
                    int newDist = vert[index].dist + airRoutes->routeCost;
                    if (newDist < vert[dest].dist) {
                        vert[dest].dist = newDist;
                        vert[dest].prev = index;
                        push(heap, dest, newDist);
                    }
                }
            }
            airRoutes = airRoutes->next;
        }
    }

    if (result == -1) {
        if (vert[endIndex].dist != INT_MAX) {
            result = vert[endIndex].dist;
        }
    }

    if (result >= 0) {
        saveCache(mapCache, cacheKey, result, matrixSize);
    }
    free(vert);
    destroy(heap);
    return result;
}

int main(int argc, const char *argv[]) {
    char command[100];
    HashMapPosition* map = NULL;
    HashMapAiroute* mapAiroute = NULL;
    HashMapCache* mapCache = NULL;
    int precX = 0, precY = 0, x = 0, y = 0, v = 0, ray = 0, initial = 0;
    while (!feof(stdin)) {
        if (scanf("%s", command)!=1) {
            deletePos(map);
            deleteAiroute(mapAiroute, precX * precY);
            deleteCache(mapCache, precX * precY);
            return 0;
        }
        if (strcmp(command, "init") == 0) {
            if (initial == 1) {
                deletePos(map);
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
            map = init(precX, precY);
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
            int res = travel_cost((Coord){x, y}, (Coord){v, ray}, map, mapAiroute, mapCache, precX * precY);
            printf("%d\n", res);
        }
    }
    deletePos(map);
    deleteAiroute(mapAiroute, precX * precY);
    deleteCache(mapCache, precX * precY);
    return 0;
}