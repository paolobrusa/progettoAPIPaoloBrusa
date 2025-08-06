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

typedef struct {
    int dist;
    int prev;
    int visited;
} Vert;

typedef struct {
    int *data;
    int *p;
    int len;
} Heap;

static unsigned int pow2(unsigned int n) {
    if (n <= 1) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

static unsigned int hashing(Coord coord, int matrixSizeAir2) {
    unsigned int hash = coord.x * 31 + coord.y;
    return hash & (matrixSizeAir2 - 1);
}

static unsigned int hashingDouble(DoubleCoord coord, int matrixSize2) {
    unsigned int hash = (coord.x * 31 + coord.y) * 31 + (coord.z * 31 + coord.w);
    return hash & (matrixSize2 - 1);
}

static int checkKey(Coord a, Coord b) {
    return (a.x == b.x) && (a.y == b.y);
}

static int checkKeyDouble(DoubleCoord a, DoubleCoord b) {
    return (a.x == b.x) && (a.y == b.y) && (a.z == b.z) && (a.w == b.w);
}

static float getPair(HashMapPosition* map, Coord key);

static HashMapPosition* init(int raw, int columns) {
    int matrixSize = columns * raw;
    HashMapPosition* map = malloc(sizeof(HashMapPosition));
    map->key = (Coord){raw, columns};
    map->value = malloc(matrixSize * sizeof(int));
    for (int i = 0; i < matrixSize; i++) {
        map->value[i] = 1;
    }
    return map;
}

static HashMapAiroute* create2(int matrixSizeAir2) {
    HashMapAiroute* map = malloc(sizeof(HashMapAiroute));
    map->bucketsAir = calloc(matrixSizeAir2, sizeof(EntryAiroute*));
    return map;
}

static HashMapCache* createCache(int matrixSize2) {
    HashMapCache* map = malloc(sizeof(HashMapCache));
    map->bucketsDouble = calloc(matrixSize2, sizeof(EntryDouble*));
    return map;
}

static int calculateRouteCost(HashMapAiroute* map, HashMapPosition* mapPosition, Coord key, int matrixSizeAir2) {
    float dcost = getPair(mapPosition, key);
    if (dcost == 0.5f) {
        return -1;
    }
    unsigned int index = hashing(key, matrixSizeAir2);
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

static int toggle_air_route(HashMapAiroute* map, Coord key, Coord value, HashMapPosition* mapPosition, int matrixSizeAir2) {
    if (key.x < 0 || key.y < 0 || key.x >= mapPosition->key.x || key.y >= mapPosition->key.y ||
       value.x < 0 || value.y < 0 || value.x >= mapPosition->key.x || value.y >= mapPosition->key.y) {
        return 1;
       }
    if (getPair(mapPosition, key) == 0.5f || getPair(mapPosition, value) == 0.5f) {
        return 1;
    }
    int i=0;
    unsigned int index = hashing(key, matrixSizeAir2);
    EntryAiroute* entry = map->bucketsAir[index];
    while (entry != NULL) {
        if (checkKey(entry->key, key)) {
            ValueAir *entryCopy = entry->value;
            ValueAir *temp = NULL;
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
                newEntry->routeCost = calculateRouteCost(map, mapPosition, key, matrixSizeAir2);
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
    entry->value->routeCost = calculateRouteCost(map, mapPosition, key, matrixSizeAir2);
    entry->value->next = NULL;
    entry->next = map->bucketsAir[index];
    map->bucketsAir[index] = entry;
    return 0;
}

static void saveCache(HashMapCache* cache, DoubleCoord coord, int cost, int matrixSize2) {
    unsigned int index = hashingDouble(coord, matrixSize2);
    EntryDouble* entry = malloc(sizeof(EntryDouble));
    entry->key = coord;
    entry->value = cost;
    entry->next = cache->bucketsDouble[index];
    cache->bucketsDouble[index] = entry;
}

static int getCache(HashMapCache* cache, DoubleCoord coord, int matrixSize2) {
    unsigned int index = hashingDouble(coord, matrixSize2);
    EntryDouble* entry = cache->bucketsDouble[index];
    while (entry != NULL) {
        if (checkKeyDouble(entry->key, coord)) {
            return entry->value;
        }
        entry = entry->next;
    }
    return -2;
}

static void modifyValue(HashMapPosition* map, Coord key, int value) {
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

static float getPair(HashMapPosition* map, Coord key) {
    if (key.x < 0 || key.y < 0 || key.x >= map->key.x || key.y >= map->key.y) {
        return 0.5f;
    }
    return map->value[key.x * map->key.y + key.y];
}

static void deletePos(HashMapPosition* map) {
    free(map->value);
    free(map);
}

static void deleteAiroute(HashMapAiroute* map, int matrixSizeAir2) {
    for (int i = 0; i < matrixSizeAir2; i++) {
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

static void deleteCache(HashMapCache* cache, int matrixSize2) {
    for (int i = 0; i < matrixSize2; i++) {
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

static Cube newCube(Cube a, Cube b) {
    Cube cube;
    cube.q = a.q - b.q;
    cube.r = a.r - b.r;
    cube.s = a.s - b.s;
    return cube;
}

static int distEsagoni(Coord a, Coord b) {
    int q, r, s, q1, r1, s1;
    q = a.x - ((a.y - (a.y & 1)) >> 1);
    r = a.y;
    s = -q - r;
    q1 = b.x - ((b.y - (b.y & 1)) >> 1);
    r1 = b.y;
    s1 = -q1 - r1;
    Cube cube = newCube((Cube){q,r,s}, (Cube){q1,r1,s1});
    int aq = (cube.q < 0) ? -cube.q : cube.q, ar = (cube.r < 0) ? -cube.r : cube.r, as = (cube.s < 0) ? -cube.s : cube.s;
    int max = (aq > ar) ? ((aq > as) ? aq : as) : ((ar > as) ? ar : as);
    return max;
}

static void modifyAirRoutesCost(HashMapAiroute* mapAiroute, Coord key, int costChange, int matrixSizeAir2) {
    unsigned int index = hashing(key, matrixSizeAir2);
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

static int change_cost(Coord key, int value, int ray, HashMapPosition* mapPosition, HashMapAiroute* mapAiroute, int matrixSizeAir2) {
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
    int minx = key.x-ray;;
    int maxx = key.x+ray;
    int miny = key.y-ray;
    int maxy = key.y+ray;
    for (int i = minx; i <= maxx; i++) {
        for (int j = miny; j <= maxy; j++) {
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
                modifyAirRoutesCost(mapAiroute, (Coord){i,j}, dcost, matrixSizeAir2);
            }
        }
    }
    return 0;
}

static Heap *createHeap(int n) {
    Heap *h = calloc(1, sizeof(Heap));
    h->data = calloc(n + 1, sizeof(int));
    h->p = calloc(n + 1, sizeof(int));
    return h;
}

static void push(Heap *h, int v, int p) {
    h->len++;
    int i = h->len;
    int j = i >> 1;
    while (i > 1 && h->p[j] > p) {
        h->data[i] = h->data[j];
        h->p[i] = h->p[j];
        i = j;
        j = i >> 1;
    }
    h->data[i] = v;
    h->p[i] = p;
}

static int pop(Heap *h) {
    int v = h->data[1];
    int len = h->len;
    int lenp = h->p[len];
    int i = 1;

    while (1) {
        int left = i << 1;
        int right = left + 1;
        int j = len;
        if (left <= len && h->p[left] < lenp)
            j = left;
        if (right <= len && h->p[right] < lenp)
            j = right;
        if (j == len) break;
        h->data[i] = h->data[j];
        h->p[i] = h->p[j];
        i = j;
    }
    h->data[i] = h->data[len];
    h->p[i] = lenp;
    h->len--;
    return v;
}

static void destroy(Heap *h) {
    free(h->data);
    free(h->p);
    free(h);
}

static ValueAir* getAirRoutes(HashMapAiroute* mapAiroute, Coord pos, int matrixSizeAir2) {
    unsigned int index = hashing(pos, matrixSizeAir2);
    EntryAiroute* entry = mapAiroute->bucketsAir[index];
    while (entry != NULL) {
        if (checkKey(entry->key, pos)) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

static int travel_cost(Coord start, Coord end, HashMapPosition* mapPosition,HashMapAiroute* mapAiroute, HashMapCache* mapCache, int matrixSize, int matrixSizeAir2, int matrixSize2) {
    int rows = mapPosition->key.x;
    int cols = mapPosition->key.y;

    if (start.x < 0 || start.x >= rows || start.y < 0 || start.y >= cols ||
        end.x < 0 || end.x >= rows || end.y < 0 || end.y >= cols) {
        return -1;
    }

    if (start.x == end.x && start.y == end.y) {
        return 0;
    }

    DoubleCoord cacheKey = {start.x, start.y, end.x, end.y};
    int cachedResult = getCache(mapCache, cacheKey, matrixSize2);
    if (cachedResult != -2) {
        return cachedResult;
    }

    int startIndex = start.x * cols + start.y;
    int endIndex = end.x * cols + end.y;

    float startCostFloat = getPair(mapPosition, start);
    float endCostFloat = getPair(mapPosition, end);
    if (startCostFloat == 0.5f || endCostFloat == 0.5f || startCostFloat == 0.0f) {
        return -1;
    }
    Vert *vert = calloc(matrixSize, sizeof(Vert));
    for (int i = 0; i < matrixSize; i++) {
        vert[i].dist = INT_MAX;
        vert[i].visited = 0;
        vert[i].prev = -1;
    }
    vert[startIndex].dist = 0;
    Heap *heap = createHeap(matrixSize);
    push(heap, startIndex, 0);

    int result = -1;
    //int distance = distEsagoni(start, end) * 4;

    while (heap->len > 0) {
        int index = pop(heap);
        if (vert[index].visited) continue;
        vert[index].visited = 1;
        Coord pos = (Coord){index / cols, index % cols};
        if (pos.x == end.x && pos.y == end.y) {
            result = vert[index].dist;
            break;
        }
        // if (vert[index].dist > distance) continue;
        float costExit = getPair(mapPosition, pos);
        if (costExit == 0.5f || costExit == 0.0f) continue;

        int cost = (int)costExit;

        static Coord offsetEven[6] = {{+1,0}, {0,-1}, {-1, -1}, {-1,0}, {-1,+1}, {0, +1}};
        static Coord offsetOdd[6] = {{+1,0}, {+1,-1}, {0, -1}, {-1,0}, {0,+1}, {+1, +1}};

        const Coord* offset = (pos.y % 2 == 0) ? offsetEven : offsetOdd;

        for (int i = 0; i < 6; i++) {
            int x = pos.x + offset[i].x;
            int y = pos.y + offset[i].y;
            if (x < 0 || x >= rows || y < 0 || y >= cols) continue;
            int hex = x * cols + y;
            if (vert[hex].visited) continue;
            float hexCost = getPair(mapPosition, (Coord){x,y});
            if (hexCost == 0.5f) continue;
            int newDist = vert[index].dist + cost;
            // if (newDist > distance) continue;
            if (newDist < vert[hex].dist) {
                vert[hex].dist = newDist;
                vert[hex].prev = index;
                push(heap, hex, newDist);
            }
        }

        ValueAir* airRoutes = getAirRoutes(mapAiroute, pos, matrixSizeAir2);
        while (airRoutes != NULL) {
            Coord airDest = airRoutes->value1;
            int dest = airDest.x * cols + airDest.y;

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
        saveCache(mapCache, cacheKey, result, matrixSize2);
    }
    free(vert);
    destroy(heap);
    return result;
}

int main(int argc, const char *argv[]) {
    HashMapPosition* map = NULL;
    HashMapAiroute* mapAiroute = NULL;
    HashMapCache* mapCache = NULL;
    int precX = 0, precY = 0, x = 0, y = 0, v = 0, ray = 0, initial = 0, matrixSize = 0, matrixSizeAir2 = 0, matrixSize2 = 0;
    while (!feof(stdin)) {
        char command[20];
        if (scanf("%s", command)!=1) {
            deletePos(map);
            deleteAiroute(mapAiroute, matrixSizeAir2);
            deleteCache(mapCache, matrixSize2);
            return 0;
        }
        if (command[0] == 'i') {
            if (initial == 1) {
                deletePos(map);
                deleteAiroute(mapAiroute, matrixSizeAir2);
                deleteCache(mapCache, matrixSize2);
                initial = 0;
            }
            if(scanf("%d", &precX) != 1) {
                continue;
            }
            if (scanf("%d", &precY) != 1) {
                continue;
            }
            puts("OK");
            initial = 1;
            matrixSize = precX * precY;
            map = init(precX, precY);
            matrixSizeAir2 = pow2(matrixSize);
            matrixSize2 = pow2(matrixSize);
            mapAiroute = create2(matrixSizeAir2);
            mapCache = createCache(matrixSize2);
        }
        else if (command[0] == 'c') {
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
                puts("KO");
                continue;
            }
            if (change_cost((Coord){x,y}, v, ray, map, mapAiroute, matrixSizeAir2) == 0) {
                puts("OK");
                deleteCache(mapCache, matrixSize2);
                mapCache = createCache(matrixSize2);
            }
            else {
                puts("KO");
            }
        }
        else if (command[1] == 'o') {
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
                puts("KO");
                continue;
            }
            if (toggle_air_route(mapAiroute, (Coord){x,y}, (Coord){v,ray}, map, matrixSizeAir2) == 0) {
                puts("OK");
                deleteCache(mapCache, matrixSize2);
                mapCache = createCache(matrixSize2);
            }
            else {
                puts("KO");
            }
        }
        else if (command[0] == 't') {
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
                puts("-1");
                continue;
            }
            int res = travel_cost((Coord){x, y}, (Coord){v, ray}, map, mapAiroute, mapCache, matrixSize, matrixSizeAir2, matrixSize2);
            printf("%d\n", res);
        }
    }
    deletePos(map);
    deleteAiroute(mapAiroute, matrixSizeAir2);
    deleteCache(mapCache, matrixSize2);
    return 0;
}