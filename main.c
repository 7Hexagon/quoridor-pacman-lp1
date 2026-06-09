// Quoridor Pac-Man - Trabajo Practico LP1
// Implementacion en C con raylib.
//
// Controles principales:
// - Menu: flechas arriba/abajo o D-Pad para elegir opcion, izquierda/derecha o D-Pad para cambiar.
// - Enter o X/Cross: iniciar partida.
// - E o R1: editor de mapas.
// - Juego: flechas/D-Pad para mover, M/Triangulo para modo muro,
// Enter/X/Options para terminar turno, R/Cuadrado para reiniciar.
// - Editor: flechas/D-Pad mueven cursor, P coloca Pac-Man, 1..4 fantasmas,
// Q/W/E/R pac-bolas, H/V muros permanentes, S/L1 guardar.



// ============================================================
// SECCION 1: Librerias y constantes
// ============================================================

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>

#define SCREEN_W 1100
#define SCREEN_H 720

#define MAX_GHOSTS 4
#define MAX_PELLETS 4
#define MIN_BOARD 5
#define MAX_BOARD 18
#define DEFAULT_ROWS 9
#define DEFAULT_COLS 9
#define MAX_CONFIG_ITEMS 16
#define TEXT_BUFFER 160
#define MAX_GAMEPADS 8

#define OWNER_PAC 0
#define OWNER_GHOSTS 1
#define OWNER_PERMANENT -1

#define WIN_NONE 0
#define WIN_PAC 1
#define WIN_GHOSTS 2

#define MAP_COUNT 4


// ============================================================
// SECCION 2: Archivos, nombres y recursos
// ============================================================

const char *MAP_FILES[MAP_COUNT] = {
    "maps/mapa_clasico_9x9.map",
    "maps/mapa_pasillos_7x11.map",
    "maps/mapa_lab_11x9.map",
    "maps/custom_map.txt"
};

const char *MAP_NAMES[MAP_COUNT] = {
    "Clasico 9x9",
    "Pasillos 7x11",
    "Laberinto 11x9",
    "Mapa del editor"
};

const char *GHOST_NAMES[MAX_GHOSTS] = {"Blinky", "Inky", "Pinky", "Clyde"};
Color GHOST_COLORS[MAX_GHOSTS];

#define MUSIC_BACKGROUND "assets/music/background.wav"
#define SOUND_EAT "assets/sounds/eat.wav"
#define SOUND_DEATH "assets/sounds/death.wav"

typedef struct {
    Music background;
    Sound eat;
    Sound death;

    bool hasBackground;
    bool hasEat;
    bool hasDeath;
} AudioBank;

static AudioBank audio = {0};


// ============================================================
// SECCION 3: Mapas predeterminados
// ============================================================

const char *DEFAULT_MAP_1 =
"NAME Clasico 9x9\n"
"SIZE 9 9\n"
"PAC 8 4\n"
"GHOST 0 0 4\n"
"GHOST 1 0 0\n"
"GHOST 2 0 8\n"
"GHOST 3 4 4\n"
"PELLET 0 1 1\n"
"PELLET 1 1 7\n"
"PELLET 2 7 1\n"
"PELLET 3 7 7\n"
"WALL 2 1 2 2\n"
"WALL 2 2 3 2\n"
"WALL 2 6 2 7\n"
"WALL 2 6 3 6\n"
"WALL 4 3 4 4\n"
"WALL 4 4 4 5\n"
"WALL 5 4 6 4\n"
"WALL 6 1 6 2\n"
"WALL 6 6 6 7\n"
"END\n";

const char *DEFAULT_MAP_2 =
"NAME Pasillos 7x11\n"
"SIZE 7 11\n"
"PAC 6 5\n"
"GHOST 0 0 5\n"
"GHOST 1 0 1\n"
"GHOST 2 0 9\n"
"GHOST 3 3 5\n"
"PELLET 0 1 1\n"
"PELLET 1 1 9\n"
"PELLET 2 5 1\n"
"PELLET 3 5 9\n"
"WALL 1 3 2 3\n"
"WALL 2 3 3 3\n"
"WALL 3 3 4 3\n"
"WALL 1 7 2 7\n"
"WALL 2 7 3 7\n"
"WALL 3 7 4 7\n"
"WALL 3 1 3 2\n"
"WALL 3 8 3 9\n"
"WALL 5 4 5 5\n"
"WALL 5 5 5 6\n"
"END\n";

const char *DEFAULT_MAP_3 =
"NAME Laberinto 11x9\n"
"SIZE 11 9\n"
"PAC 10 4\n"
"GHOST 0 0 4\n"
"GHOST 1 0 0\n"
"GHOST 2 0 8\n"
"GHOST 3 5 4\n"
"PELLET 0 1 1\n"
"PELLET 1 1 7\n"
"PELLET 2 9 1\n"
"PELLET 3 9 7\n"
"WALL 1 2 1 3\n"
"WALL 1 5 1 6\n"
"WALL 2 2 3 2\n"
"WALL 2 6 3 6\n"
"WALL 3 3 3 4\n"
"WALL 3 4 3 5\n"
"WALL 5 0 5 1\n"
"WALL 5 7 5 8\n"
"WALL 6 2 7 2\n"
"WALL 6 6 7 6\n"
"WALL 8 3 8 4\n"
"WALL 8 4 8 5\n"
"END\n";


// ============================================================
// SECCION 4: Tipos de datos principales
// ============================================================

typedef enum {
    SCREEN_CONFIG = 0,
    SCREEN_GAME,
    SCREEN_EDITOR,
    SCREEN_GAMEOVER
} ScreenState;

typedef enum {
    MODE_AI = 0,
    MODE_PVP = 1
} GameMode;

typedef enum {
    TEAM_PAC = 0,
    TEAM_GHOSTS = 1
} Team;

typedef struct {
    int r;
    int c;
} Pos;

typedef struct {
    Pos a;
    Pos b;
    int owner;
    int life;
} Wall;

typedef struct {
    int rows;
    int cols;
    Pos pacStart;
    Pos ghostStart[MAX_GHOSTS];
    Pos pelletStart[MAX_PELLETS];
    Wall *walls;
    int wallCount;
    int wallCap;
    char name[64];
} MapData;

typedef struct {
    GameMode mode;
    int selectedMap;
    int ghostEnabled[MAX_GHOSTS];
    int ghostDifficulty[MAX_GHOSTS];
    int pacWallHandInit;
    int ghostWallHandInit;
    int wallLife;
} Config;

typedef struct {
    Config cfg;
    MapData map;

    Pos pac;
    Pos ghost[MAX_GHOSTS];
    int ghostAlive[MAX_GHOSTS];
    int pelletEaten[MAX_PELLETS];

    int lives;
    int pelletsEaten;
    int pacWalls;
    int ghostWalls;

    Wall *tempWalls;
    int tempCount;
    int tempCap;

    Team currentTeam;
    int currentGhost;
    int actionsLeft;
    int globalTurn;
    int winner;

    bool wallMode;
    bool pacPoweredThisTurn;
    double aiTimer;
    char message[TEXT_BUFFER];
} GameState;

typedef struct {
    ScreenState screen;
    Config cfg;
    GameState game;
    MapData editorMap;
    Pos editorCursor;
    int configCursor;
    char status[TEXT_BUFFER];
} App;


// ============================================================
// SECCION 5: Declaracion de funciones
// ============================================================

static int clampInt(int v, int lo, int hi);
static bool posEq(Pos a, Pos b);
static bool insideMap(const MapData *m, Pos p);
static bool adjacent(Pos a, Pos b);
static Wall makeWall(Pos a, Pos b, int owner, int life);
static bool sameEdge(Wall w, Pos a, Pos b);
static bool addWallDynamic(Wall **arr, int *count, int *cap, Wall wall);
static bool removeWallAt(Wall *arr, int *count, int index);
static int findWall(Wall *arr, int count, Pos a, Pos b);

static void initMap(MapData *m, int rows, int cols, const char *name);
static void freeMap(MapData *m);
static void copyMap(MapData *dst, const MapData *src);
static bool loadMapFile(const char *filename, MapData *out);
static bool saveMapFile(const char *filename, const MapData *m);
static void ensureDefaultMaps(void);
static void createBlankEditorMap(MapData *m);
static void cleanMapAfterResize(MapData *m);

static void initDefaultConfig(Config *cfg);
static void freeGame(GameState *g);
static bool startGame(GameState *g, const Config *cfg, const char *mapFile);
static void restartMatch(GameState *g);
static void resetAfterCapture(GameState *g);
static int nextEnabledAliveGhost(const GameState *g, int start);
static void beginPacTurn(GameState *g);
static void beginGhostTurns(GameState *g);
static void finishGhostTurn(GameState *g);
static void endPacTurn(GameState *g);
static void tickTempWalls(GameState *g);
static bool isBlocked(const GameState *g, Pos a, Pos b);
static bool isBlockedOnMapOnly(const MapData *m, Pos a, Pos b);
static bool ghostCanSeePac(const GameState *g, int ghostIndex);
static int moveStepsForGhost(const GameState *g, int ghostIndex);
static bool tryMovePac(GameState *g, int dr, int dc);
static bool tryMoveGhost(GameState *g, int ghostIndex, int dr, int dc);
static bool tryPlaceTempWall(GameState *g, Pos from, Pos to, int owner);
static void checkPelletAtPac(GameState *g);
static void checkPacOnGhost(GameState *g);
static void checkGhostOnPac(GameState *g, int ghostIndex);
static void setWinnerIfNeeded(GameState *g);
static const char *levelName(int eaten);



// ============================================================
// SECCION 6: Entrada por teclado y gamepad
// ============================================================

static bool textContainsIgnoreCase(const char *text, const char *needle)
{
    if (text == NULL || needle == NULL) return false;
    if (needle[0] == '\0') return true;

    for (int i = 0; text[i] != '\0'; i++) {
        int j = 0;
        while (text[i + j] != '\0' && needle[j] != '\0' &&
               tolower((unsigned char)text[i + j]) == tolower((unsigned char)needle[j])) {
            j++;
        }
        if (needle[j] == '\0') return true;
    }

    return false;
}

static bool isIgnoredGamepadName(const char *name)
{
    if (name == NULL) return false;

    // VirtualBox a veces mete dispositivos raros como "USB Tablet".
    // Los ignoramos para que raylib elija el control real.
    return textContainsIgnoreCase(name, "virtualbox") ||
           textContainsIgnoreCase(name, "usb tablet") ||
           textContainsIgnoreCase(name, "tablet") ||
           textContainsIgnoreCase(name, "mouse");
}

static int activeGamepad(void)
{
    int fallback = -1;

    for (int i = 0; i < MAX_GAMEPADS; i++) {
        if (IsGamepadAvailable(i)) {
            const char *name = GetGamepadName(i);
            if (fallback < 0) fallback = i;

            if (!isIgnoredGamepadName(name)) {
                return i;
            }
        }
    }

    return fallback;
}

static const char *activeGamepadName(void)
{
    static char text[128];
    int gp = activeGamepad();

    if (gp < 0) return "Control: no detectado";

    const char *name = GetGamepadName(gp);
    snprintf(text, sizeof(text), "Control %d: %s", gp, (name != NULL && name[0] != '\0') ? name : "desconocido");
    return text;
}

static bool padPressed(int button)
{
    int gp = activeGamepad();
    return gp >= 0 && IsGamepadButtonPressed(gp, button);
}

static bool inputUpPressed(void)
{
    return IsKeyPressed(KEY_UP) || padPressed(GAMEPAD_BUTTON_LEFT_FACE_UP);
}

static bool inputDownPressed(void)
{
    return IsKeyPressed(KEY_DOWN) || padPressed(GAMEPAD_BUTTON_LEFT_FACE_DOWN);
}

static bool inputLeftPressed(void)
{
    return IsKeyPressed(KEY_LEFT) || padPressed(GAMEPAD_BUTTON_LEFT_FACE_LEFT);
}

static bool inputRightPressed(void)
{
    return IsKeyPressed(KEY_RIGHT) || padPressed(GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
}

static bool inputConfirmPressed(void)
{
    return IsKeyPressed(KEY_ENTER) ||
           padPressed(GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||  // X / Cross
           padPressed(GAMEPAD_BUTTON_MIDDLE_RIGHT);       // Options / Start
}

static bool inputBackPressed(void)
{
    return IsKeyPressed(KEY_ESCAPE) ||
           padPressed(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT) || // Circle
           padPressed(GAMEPAD_BUTTON_MIDDLE_LEFT);        // Create / Select
}

static bool inputWallPressed(void)
{
    return IsKeyPressed(KEY_M) ||
           padPressed(GAMEPAD_BUTTON_RIGHT_FACE_UP);      // Triangle
}

static bool inputRestartPressed(void)
{
    return IsKeyPressed(KEY_R) ||
           padPressed(GAMEPAD_BUTTON_RIGHT_FACE_LEFT);    // Square
}

static bool inputEditorPressed(void)
{
    return IsKeyPressed(KEY_E) ||
           padPressed(GAMEPAD_BUTTON_RIGHT_TRIGGER_1);    // R1
}

static bool inputSavePressed(void)
{
    return IsKeyPressed(KEY_S) ||
           padPressed(GAMEPAD_BUTTON_LEFT_TRIGGER_1);     // L1
}

static bool readDirectionPressed(int *dr, int *dc)
{
    *dr = 0;
    *dc = 0;

    if (inputUpPressed()) {
        *dr = -1;
        return true;
    }
    if (inputDownPressed()) {
        *dr = 1;
        return true;
    }
    if (inputLeftPressed()) {
        *dc = -1;
        return true;
    }
    if (inputRightPressed()) {
        *dc = 1;
        return true;
    }

    return false;
}

static void updateConfigScreen(App *app);
static void renderConfigScreen(const App *app);
static void updateGameScreen(App *app);
static void renderGameScreen(const App *app);
static void updateEditorScreen(App *app);
static void renderEditorScreen(const App *app);
static void renderGameOverScreen(const App *app);
static void renderBoardGame(const GameState *g, int ox, int oy, int cell);
static void renderBoardEditor(const MapData *m, Pos cursor, int ox, int oy, int cell);
static int computeCellSize(int rows, int cols, int availableW, int availableH);
static void drawWallLine(Wall w, int ox, int oy, int cell, Color color, int thickness, bool drawLife);
static void drawHud(const GameState *g, int x, int y);
static void updateAI(GameState *g);
static bool aiTryPlaceWall(GameState *g, int ghostIndex);
static void aiMoveGhost(GameState *g, int ghostIndex);
static void changeConfigValue(Config *cfg, int item, int delta);
static const char *configItemText(const Config *cfg, int item);
static int configItemCount(void);

static bool textContainsIgnoreCase(const char *text, const char *needle);
static bool isIgnoredGamepadName(const char *name);
static int activeGamepad(void);
static const char *activeGamepadName(void);
static bool padPressed(int button);
static bool inputUpPressed(void);
static bool inputDownPressed(void);
static bool inputLeftPressed(void);
static bool inputRightPressed(void);
static bool inputConfirmPressed(void);
static bool inputBackPressed(void);
static bool inputWallPressed(void);
static bool inputRestartPressed(void);
static bool inputEditorPressed(void);
static bool inputSavePressed(void);
static bool readDirectionPressed(int *dr, int *dc);

static void initGameAudio(void);
static void updateGameAudio(void);
static void closeGameAudio(void);
static void playEatSound(void);
static void playDeathSound(void);


// ============================================================
// SECCION 7: Audio
// ============================================================

static void initGameAudio(void)
{
    InitAudioDevice();

    if (!IsAudioDeviceReady()) {
        return;
    }

    if (FileExists(MUSIC_BACKGROUND)) {
        audio.background = LoadMusicStream(MUSIC_BACKGROUND);
        audio.background.looping = true;
        audio.hasBackground = true;
        SetMusicVolume(audio.background, 0.35f);
        PlayMusicStream(audio.background);
    }

    if (FileExists(SOUND_EAT)) {
        audio.eat = LoadSound(SOUND_EAT);
        audio.hasEat = true;
        SetSoundVolume(audio.eat, 0.75f);
    }

    if (FileExists(SOUND_DEATH)) {
        audio.death = LoadSound(SOUND_DEATH);
        audio.hasDeath = true;
        SetSoundVolume(audio.death, 0.90f);
    }
}

static void updateGameAudio(void)
{
    if (audio.hasBackground) {
        UpdateMusicStream(audio.background);
    }
}

static void closeGameAudio(void)
{
    if (audio.hasEat) UnloadSound(audio.eat);
    if (audio.hasDeath) UnloadSound(audio.death);
    if (audio.hasBackground) UnloadMusicStream(audio.background);

    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
}

static void playEatSound(void)
{
    if (audio.hasEat) {
        PlaySound(audio.eat);
    }
}

static void playDeathSound(void)
{
    if (audio.hasDeath) {
        PlaySound(audio.death);
    }
}


// ============================================================
// SECCION 8: Utilidades generales
// ============================================================

static int clampInt(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static bool posEq(Pos a, Pos b)
{
    return a.r == b.r && a.c == b.c;
}

static bool insideMap(const MapData *m, Pos p)
{
    return p.r >= 0 && p.r < m->rows && p.c >= 0 && p.c < m->cols;
}

static bool adjacent(Pos a, Pos b)
{
    int dr = a.r - b.r;
    int dc = a.c - b.c;
    if (dr < 0) dr = -dr;
    if (dc < 0) dc = -dc;
    return (dr + dc) == 1;
}

static Wall makeWall(Pos a, Pos b, int owner, int life)
{
    Wall w;
    w.a = a;
    w.b = b;
    w.owner = owner;
    w.life = life;
    return w;
}

static bool sameEdge(Wall w, Pos a, Pos b)
{
    return (posEq(w.a, a) && posEq(w.b, b)) || (posEq(w.a, b) && posEq(w.b, a));
}

static bool addWallDynamic(Wall **arr, int *count, int *cap, Wall wall)
{
    if (*count >= *cap) {
        int newCap = (*cap == 0) ? 16 : (*cap * 2);
        Wall *tmp = (Wall *)realloc(*arr, (size_t)newCap * sizeof(Wall));
        if (tmp == NULL) return false;
        *arr = tmp;
        *cap = newCap;
    }

    (*arr)[*count] = wall;
    (*count)++;
    return true;
}

static bool removeWallAt(Wall *arr, int *count, int index)
{
    if (index < 0 || index >= *count) return false;
    for (int i = index; i < *count - 1; i++) {
        arr[i] = arr[i + 1];
    }
    (*count)--;
    return true;
}

static int findWall(Wall *arr, int count, Pos a, Pos b)
{
    for (int i = 0; i < count; i++) {
        if (sameEdge(arr[i], a, b)) return i;
    }
    return -1;
}


// ============================================================
// SECCION 9: Mapas y archivos
// ============================================================

static void initMap(MapData *m, int rows, int cols, const char *name)
{
    memset(m, 0, sizeof(MapData));
    m->rows = clampInt(rows, MIN_BOARD, MAX_BOARD);
    m->cols = clampInt(cols, MIN_BOARD, MAX_BOARD);
    m->pacStart = (Pos){m->rows - 1, m->cols / 2};

    m->ghostStart[0] = (Pos){0, m->cols / 2};
    m->ghostStart[1] = (Pos){0, 0};
    m->ghostStart[2] = (Pos){0, m->cols - 1};
    m->ghostStart[3] = (Pos){m->rows / 2, m->cols / 2};

    m->pelletStart[0] = (Pos){1, 1};
    m->pelletStart[1] = (Pos){1, m->cols - 2};
    m->pelletStart[2] = (Pos){m->rows - 2, 1};
    m->pelletStart[3] = (Pos){m->rows - 2, m->cols - 2};

    m->walls = NULL;
    m->wallCount = 0;
    m->wallCap = 0;
    strncpy(m->name, name ? name : "Mapa", sizeof(m->name) - 1);
}

static void freeMap(MapData *m)
{
    if (m->walls != NULL) {
        free(m->walls);
    }
    memset(m, 0, sizeof(MapData));
}

static void copyMap(MapData *dst, const MapData *src)
{
    initMap(dst, src->rows, src->cols, src->name);
    dst->pacStart = src->pacStart;
    for (int i = 0; i < MAX_GHOSTS; i++) dst->ghostStart[i] = src->ghostStart[i];
    for (int i = 0; i < MAX_PELLETS; i++) dst->pelletStart[i] = src->pelletStart[i];

    for (int i = 0; i < src->wallCount; i++) {
        addWallDynamic(&dst->walls, &dst->wallCount, &dst->wallCap, src->walls[i]);
    }
}

static bool loadMapFile(const char *filename, MapData *out)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL) return false;

    initMap(out, DEFAULT_ROWS, DEFAULT_COLS, filename);

    char key[64];
    while (fscanf(f, "%63s", key) == 1) {
        if (strcmp(key, "NAME") == 0) {
            char line[64];
            if (fgets(line, sizeof(line), f) != NULL) {
                int start = 0;
                while (line[start] == ' ' || line[start] == '\t') start++;
                line[strcspn(line, "\r\n")] = '\0';
                strncpy(out->name, line + start, sizeof(out->name) - 1);
            }
        } else if (strcmp(key, "SIZE") == 0) {
            int r, c;
            if (fscanf(f, "%d %d", &r, &c) == 2) {
                out->rows = clampInt(r, MIN_BOARD, MAX_BOARD);
                out->cols = clampInt(c, MIN_BOARD, MAX_BOARD);
            }
        } else if (strcmp(key, "PAC") == 0) {
            fscanf(f, "%d %d", &out->pacStart.r, &out->pacStart.c);
        } else if (strcmp(key, "GHOST") == 0) {
            int id, r, c;
            if (fscanf(f, "%d %d %d", &id, &r, &c) == 3) {
                if (id >= 0 && id < MAX_GHOSTS) out->ghostStart[id] = (Pos){r, c};
            }
        } else if (strcmp(key, "PELLET") == 0) {
            int id, r, c;
            if (fscanf(f, "%d %d %d", &id, &r, &c) == 3) {
                if (id >= 0 && id < MAX_PELLETS) out->pelletStart[id] = (Pos){r, c};
            }
        } else if (strcmp(key, "WALL") == 0) {
            Pos a, b;
            if (fscanf(f, "%d %d %d %d", &a.r, &a.c, &b.r, &b.c) == 4) {
                if (insideMap(out, a) && insideMap(out, b) && adjacent(a, b)) {
                    if (findWall(out->walls, out->wallCount, a, b) < 0) {
                        addWallDynamic(&out->walls, &out->wallCount, &out->wallCap,
                                       makeWall(a, b, OWNER_PERMANENT, -1));
                    }
                }
            }
        } else if (strcmp(key, "END") == 0) {
            break;
        } else {
            char trash[256];
            fgets(trash, sizeof(trash), f);
        }
    }

    fclose(f);
    cleanMapAfterResize(out);
    return true;
}

static bool saveMapFile(const char *filename, const MapData *m)
{
    FILE *f = fopen(filename, "w");
    if (f == NULL) return false;

    fprintf(f, "NAME %s\n", m->name[0] ? m->name : "Mapa creado");
    fprintf(f, "SIZE %d %d\n", m->rows, m->cols);
    fprintf(f, "PAC %d %d\n", m->pacStart.r, m->pacStart.c);
    for (int i = 0; i < MAX_GHOSTS; i++) {
        fprintf(f, "GHOST %d %d %d\n", i, m->ghostStart[i].r, m->ghostStart[i].c);
    }
    for (int i = 0; i < MAX_PELLETS; i++) {
        fprintf(f, "PELLET %d %d %d\n", i, m->pelletStart[i].r, m->pelletStart[i].c);
    }
    for (int i = 0; i < m->wallCount; i++) {
        fprintf(f, "WALL %d %d %d %d\n", m->walls[i].a.r, m->walls[i].a.c,
                m->walls[i].b.r, m->walls[i].b.c);
    }
    fprintf(f, "END\n");
    fclose(f);
    return true;
}

static bool fileExists(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL) return false;
    fclose(f);
    return true;
}

static void writeTextIfMissing(const char *filename, const char *text)
{
    if (fileExists(filename)) return;
    FILE *f = fopen(filename, "w");
    if (f == NULL) return;
    fputs(text, f);
    fclose(f);
}

static void ensureDefaultMaps(void)
{
    mkdir("maps", 0777);
    writeTextIfMissing(MAP_FILES[0], DEFAULT_MAP_1);
    writeTextIfMissing(MAP_FILES[1], DEFAULT_MAP_2);
    writeTextIfMissing(MAP_FILES[2], DEFAULT_MAP_3);
}

static void createBlankEditorMap(MapData *m)
{
    freeMap(m);
    initMap(m, DEFAULT_ROWS, DEFAULT_COLS, "Mapa creado");
}

static void cleanMapAfterResize(MapData *m)
{
    m->rows = clampInt(m->rows, MIN_BOARD, MAX_BOARD);
    m->cols = clampInt(m->cols, MIN_BOARD, MAX_BOARD);

    m->pacStart.r = clampInt(m->pacStart.r, 0, m->rows - 1);
    m->pacStart.c = clampInt(m->pacStart.c, 0, m->cols - 1);
    for (int i = 0; i < MAX_GHOSTS; i++) {
        m->ghostStart[i].r = clampInt(m->ghostStart[i].r, 0, m->rows - 1);
        m->ghostStart[i].c = clampInt(m->ghostStart[i].c, 0, m->cols - 1);
    }
    for (int i = 0; i < MAX_PELLETS; i++) {
        m->pelletStart[i].r = clampInt(m->pelletStart[i].r, 0, m->rows - 1);
        m->pelletStart[i].c = clampInt(m->pelletStart[i].c, 0, m->cols - 1);
    }

    int i = 0;
    while (i < m->wallCount) {
        if (!insideMap(m, m->walls[i].a) || !insideMap(m, m->walls[i].b) || !adjacent(m->walls[i].a, m->walls[i].b)) {
            removeWallAt(m->walls, &m->wallCount, i);
        } else {
            i++;
        }
    }
}


// ============================================================
// SECCION 10: Configuracion y estado de partida
// ============================================================

static void initDefaultConfig(Config *cfg)
{
    cfg->mode = MODE_AI;
    cfg->selectedMap = 0;
    cfg->pacWallHandInit = 3;
    cfg->ghostWallHandInit = 1;
    cfg->wallLife = 4;
    for (int i = 0; i < MAX_GHOSTS; i++) {
        cfg->ghostEnabled[i] = 1;
        cfg->ghostDifficulty[i] = (i == 3) ? 1 : 2;
    }
}

static void freeGame(GameState *g)
{
    freeMap(&g->map);
    if (g->tempWalls != NULL) free(g->tempWalls);
    memset(g, 0, sizeof(GameState));
}

static bool startGame(GameState *g, const Config *cfg, const char *mapFile)
{
    freeGame(g);
    g->cfg = *cfg;

    if (!loadMapFile(mapFile, &g->map)) {
        initMap(&g->map, DEFAULT_ROWS, DEFAULT_COLS, "Emergencia 9x9");
        snprintf(g->message, sizeof(g->message), "No se pudo cargar el mapa. Se uso uno basico.");
    }

    g->tempWalls = NULL;
    g->tempCount = 0;
    g->tempCap = 0;
    restartMatch(g);
    return true;
}

static void restartMatch(GameState *g)
{
    g->pac = g->map.pacStart;
    for (int i = 0; i < MAX_GHOSTS; i++) {
        g->ghost[i] = g->map.ghostStart[i];
        g->ghostAlive[i] = g->cfg.ghostEnabled[i] ? 1 : 0;
    }
    for (int i = 0; i < MAX_PELLETS; i++) g->pelletEaten[i] = 0;

    g->lives = 3;
    g->pelletsEaten = 0;
    g->pacWalls = g->cfg.pacWallHandInit;
    g->ghostWalls = g->cfg.ghostWallHandInit;
    g->tempCount = 0;
    g->currentTeam = TEAM_PAC;
    g->currentGhost = -1;
    g->actionsLeft = 2;
    g->globalTurn = 1;
    g->winner = WIN_NONE;
    g->wallMode = false;
    g->pacPoweredThisTurn = false;
    g->aiTimer = GetTime();
    snprintf(g->message, sizeof(g->message), "Partida reiniciada.");
}

static void resetAfterCapture(GameState *g)
{
    playDeathSound();

    g->pac = g->map.pacStart;
    for (int i = 0; i < MAX_GHOSTS; i++) {
        if (g->cfg.ghostEnabled[i]) {
            g->ghost[i] = g->map.ghostStart[i];
            g->ghostAlive[i] = 1;
        }
    }
    g->wallMode = false;
    g->pacPoweredThisTurn = false;

    if (g->lives <= 0) {
        g->winner = WIN_GHOSTS;
        snprintf(g->message, sizeof(g->message), "Los fantasmas ganan.");
    } else {
        snprintf(g->message, sizeof(g->message), "Pac-Man perdio una vida. Reset de posiciones.");
    }
}

static int nextEnabledAliveGhost(const GameState *g, int start)
{
    for (int i = start; i < MAX_GHOSTS; i++) {
        if (g->cfg.ghostEnabled[i] && g->ghostAlive[i]) return i;
    }
    return -1;
}

static void beginPacTurn(GameState *g)
{
    g->currentTeam = TEAM_PAC;
    g->currentGhost = -1;
    g->actionsLeft = 2;
    g->wallMode = false;
    g->pacPoweredThisTurn = false;
    if (g->winner == WIN_NONE) snprintf(g->message, sizeof(g->message), "Turno de Pac-Man.");
}

static void beginGhostTurns(GameState *g)
{
    g->currentTeam = TEAM_GHOSTS;
    g->wallMode = false;
    g->currentGhost = nextEnabledAliveGhost(g, 0);
    g->aiTimer = GetTime();

    if (g->currentGhost < 0) {
        beginPacTurn(g);
    } else {
        snprintf(g->message, sizeof(g->message), "Turno de %s.", GHOST_NAMES[g->currentGhost]);
    }
}

static void finishGhostTurn(GameState *g)
{
    if (g->winner != WIN_NONE) return;
    g->wallMode = false;
    tickTempWalls(g);
    g->globalTurn++;

    int next = nextEnabledAliveGhost(g, g->currentGhost + 1);
    if (next >= 0) {
        g->currentGhost = next;
        g->aiTimer = GetTime();
        snprintf(g->message, sizeof(g->message), "Turno de %s.", GHOST_NAMES[g->currentGhost]);
    } else {
        beginPacTurn(g);
    }
}

static void endPacTurn(GameState *g)
{
    if (g->winner != WIN_NONE) return;
    g->wallMode = false;
    g->pacPoweredThisTurn = false;
    tickTempWalls(g);
    g->globalTurn++;
    beginGhostTurns(g);
}

static void tickTempWalls(GameState *g)
{
    int i = 0;
    while (i < g->tempCount) {
        g->tempWalls[i].life--;
        if (g->tempWalls[i].life <= 0) {
            if (g->tempWalls[i].owner == OWNER_PAC) g->pacWalls++;
            else if (g->tempWalls[i].owner == OWNER_GHOSTS) g->ghostWalls++;
            removeWallAt(g->tempWalls, &g->tempCount, i);
        } else {
            i++;
        }
    }
}


// ============================================================
// SECCION 11: Reglas de movimiento, muros y capturas
// ============================================================

static bool isBlockedOnMapOnly(const MapData *m, Pos a, Pos b)
{
    if (!insideMap(m, a) || !insideMap(m, b) || !adjacent(a, b)) return true;
    return findWall(m->walls, m->wallCount, a, b) >= 0;
}

static bool isBlocked(const GameState *g, Pos a, Pos b)
{
    if (!insideMap(&g->map, a) || !insideMap(&g->map, b) || !adjacent(a, b)) return true;
    if (findWall(g->map.walls, g->map.wallCount, a, b) >= 0) return true;
    if (findWall(g->tempWalls, g->tempCount, a, b) >= 0) return true;
    return false;
}

static bool ghostCanSeePac(const GameState *g, int ghostIndex)
{
    if (!g->ghostAlive[ghostIndex]) return false;
    Pos ghost = g->ghost[ghostIndex];
    Pos pac = g->pac;

    if (ghost.r == pac.r) {
        int dc = (pac.c > ghost.c) ? 1 : -1;
        Pos cur = ghost;
        while (cur.c != pac.c) {
            Pos next = {cur.r, cur.c + dc};
            if (isBlocked(g, cur, next)) return false;
            cur = next;
        }
        return true;
    }

    if (ghost.c == pac.c) {
        int dr = (pac.r > ghost.r) ? 1 : -1;
        Pos cur = ghost;
        while (cur.r != pac.r) {
            Pos next = {cur.r + dr, cur.c};
            if (isBlocked(g, cur, next)) return false;
            cur = next;
        }
        return true;
    }

    return false;
}

static int moveStepsForGhost(const GameState *g, int ghostIndex)
{
    return ghostCanSeePac(g, ghostIndex) ? 2 : 1;
}

static bool tryMovePac(GameState *g, int dr, int dc)
{
    if (g->winner != WIN_NONE || g->currentTeam != TEAM_PAC || g->actionsLeft <= 0) return false;

    Pos next = {g->pac.r + dr, g->pac.c + dc};
    if (isBlocked(g, g->pac, next)) {
        snprintf(g->message, sizeof(g->message), "Movimiento bloqueado.");
        return false;
    }

    g->pac = next;
    g->actionsLeft--;
    checkPelletAtPac(g);
    checkPacOnGhost(g);
    setWinnerIfNeeded(g);

    if (g->actionsLeft <= 0 && g->winner == WIN_NONE) {
        snprintf(g->message, sizeof(g->message), "Sin acciones. Enter para terminar turno.");
    }
    return true;
}

static bool tryMoveGhost(GameState *g, int ghostIndex, int dr, int dc)
{
    if (g->winner != WIN_NONE || ghostIndex < 0 || ghostIndex >= MAX_GHOSTS) return false;
    if (!g->cfg.ghostEnabled[ghostIndex] || !g->ghostAlive[ghostIndex]) return false;

    int steps = moveStepsForGhost(g, ghostIndex);
    bool moved = false;

    for (int s = 0; s < steps; s++) {
        Pos next = {g->ghost[ghostIndex].r + dr, g->ghost[ghostIndex].c + dc};
        if (isBlocked(g, g->ghost[ghostIndex], next)) break;
        g->ghost[ghostIndex] = next;
        moved = true;
        checkGhostOnPac(g, ghostIndex);
        if (g->winner != WIN_NONE || posEq(g->ghost[ghostIndex], g->map.ghostStart[ghostIndex])) {
            break;
        }
        if (posEq(g->ghost[ghostIndex], g->pac)) break;
    }

    if (!moved) {
        snprintf(g->message, sizeof(g->message), "%s no pudo moverse.", GHOST_NAMES[ghostIndex]);
    }
    return moved;
}

static bool tryPlaceTempWall(GameState *g, Pos from, Pos to, int owner)
{
    if (g->winner != WIN_NONE) return false;
    if (!insideMap(&g->map, from) || !insideMap(&g->map, to) || !adjacent(from, to)) {
        snprintf(g->message, sizeof(g->message), "No se puede poner muro ahi.");
        return false;
    }
    if (findWall(g->map.walls, g->map.wallCount, from, to) >= 0 ||
        findWall(g->tempWalls, g->tempCount, from, to) >= 0) {
        snprintf(g->message, sizeof(g->message), "Ya hay un muro en ese borde.");
        return false;
    }

    if (owner == OWNER_PAC) {
        if (g->pacWalls <= 0) {
            snprintf(g->message, sizeof(g->message), "Pac-Man no tiene muros en mano.");
            return false;
        }
        g->pacWalls--;
    } else {
        if (g->ghostWalls <= 0) {
            snprintf(g->message, sizeof(g->message), "Los fantasmas no tienen muros en mano.");
            return false;
        }
        g->ghostWalls--;
    }

    Wall w = makeWall(from, to, owner, g->cfg.wallLife);
    if (!addWallDynamic(&g->tempWalls, &g->tempCount, &g->tempCap, w)) {
        snprintf(g->message, sizeof(g->message), "Error de memoria al colocar muro.");
        return false;
    }

    if (owner == OWNER_PAC) {
        g->actionsLeft--;
        snprintf(g->message, sizeof(g->message), "Pac-Man coloco un muro temporal.");
    } else {
        snprintf(g->message, sizeof(g->message), "%s coloco un muro temporal.", GHOST_NAMES[g->currentGhost]);
    }
    return true;
}

static void checkPelletAtPac(GameState *g)
{
    for (int i = 0; i < MAX_PELLETS; i++) {
        if (!g->pelletEaten[i] && posEq(g->pac, g->map.pelletStart[i])) {
            g->pelletEaten[i] = 1;
            g->pelletsEaten++;
            g->actionsLeft++;
            g->pacPoweredThisTurn = true;
            playEatSound();
            snprintf(g->message, sizeof(g->message), "Pac-Man comio pac-bola: accion extra.");
            return;
        }
    }
}

static void checkPacOnGhost(GameState *g)
{
    for (int i = 0; i < MAX_GHOSTS; i++) {
        if (g->cfg.ghostEnabled[i] && g->ghostAlive[i] && posEq(g->pac, g->ghost[i])) {
            if (g->pacPoweredThisTurn) {
                g->ghostAlive[i] = 0;
                playEatSound();
                snprintf(g->message, sizeof(g->message), "Pac-Man comio a %s.", GHOST_NAMES[i]);
            } else {
                g->lives--;
                resetAfterCapture(g);
            }
            return;
        }
    }
}

static void checkGhostOnPac(GameState *g, int ghostIndex)
{
    if (ghostIndex < 0 || ghostIndex >= MAX_GHOSTS) return;
    if (g->cfg.ghostEnabled[ghostIndex] && g->ghostAlive[ghostIndex] && posEq(g->ghost[ghostIndex], g->pac)) {
        g->lives--;
        snprintf(g->message, sizeof(g->message), "%s atrapo a Pac-Man.", GHOST_NAMES[ghostIndex]);
        resetAfterCapture(g);
    }
}

static void setWinnerIfNeeded(GameState *g)
{
    if (g->pelletsEaten >= MAX_PELLETS) {
        g->winner = WIN_PAC;
        snprintf(g->message, sizeof(g->message), "Pac-Man gana: comio las 4 pac-bolas.");
    }
    if (g->lives <= 0) {
        g->winner = WIN_GHOSTS;
        snprintf(g->message, sizeof(g->message), "Los fantasmas ganan.");
    }
}

static const char *levelName(int eaten)
{
    switch (eaten) {
        case 1: return "Pac-Man novato";
        case 2: return "Pac-Man prometedor";
        case 3: return "Pac-Man de categoria";
        case 4: return "Pac-Man de elite";
        default: return "Sin nivel";
    }
}


// ============================================================
// SECCION 12: Pantalla de configuracion
// ============================================================

static int configItemCount(void)
{
    return 5 + MAX_GHOSTS * 2;
}

static const char *configItemText(const Config *cfg, int item)
{
    static char text[128];
    if (item == 0) snprintf(text, sizeof(text), "Modo de juego: %s", cfg->mode == MODE_AI ? "IA vs Player" : "Player vs Player");
    else if (item == 1) snprintf(text, sizeof(text), "Mapa: %s", MAP_NAMES[cfg->selectedMap]);
    else if (item == 2) snprintf(text, sizeof(text), "Muros mano Pac-Man: %d", cfg->pacWallHandInit);
    else if (item == 3) snprintf(text, sizeof(text), "Muros mano fantasmas: %d", cfg->ghostWallHandInit);
    else if (item == 4) snprintf(text, sizeof(text), "Vida de muros temporales: %d turnos", cfg->wallLife);
    else {
        int base = item - 5;
        int ghost = base / 2;
        if (base % 2 == 0) {
            snprintf(text, sizeof(text), "%s habilitado: %s", GHOST_NAMES[ghost], cfg->ghostEnabled[ghost] ? "Si" : "No");
        } else {
            snprintf(text, sizeof(text), "%s dificultad IA: %d", GHOST_NAMES[ghost], cfg->ghostDifficulty[ghost]);
        }
    }
    return text;
}

static void changeConfigValue(Config *cfg, int item, int delta)
{
    if (item == 0) {
        cfg->mode = (cfg->mode == MODE_AI) ? MODE_PVP : MODE_AI;
    } else if (item == 1) {
        cfg->selectedMap = (cfg->selectedMap + delta + MAP_COUNT) % MAP_COUNT;
    } else if (item == 2) {
        cfg->pacWallHandInit = clampInt(cfg->pacWallHandInit + delta, 0, 10);
    } else if (item == 3) {
        cfg->ghostWallHandInit = clampInt(cfg->ghostWallHandInit + delta, 0, 10);
    } else if (item == 4) {
        cfg->wallLife = clampInt(cfg->wallLife + delta, 1, 12);
    } else {
        int base = item - 5;
        int ghost = base / 2;
        if (ghost >= 0 && ghost < MAX_GHOSTS) {
            if (base % 2 == 0) cfg->ghostEnabled[ghost] = !cfg->ghostEnabled[ghost];
            else cfg->ghostDifficulty[ghost] = clampInt(cfg->ghostDifficulty[ghost] + delta, 1, 3);
        }
    }
}

static void updateConfigScreen(App *app)
{
    if (inputDownPressed()) app->configCursor = (app->configCursor + 1) % configItemCount();
    if (inputUpPressed()) app->configCursor = (app->configCursor - 1 + configItemCount()) % configItemCount();
    if (inputLeftPressed()) changeConfigValue(&app->cfg, app->configCursor, -1);
    if (inputRightPressed()) changeConfigValue(&app->cfg, app->configCursor, 1);

    if (inputConfirmPressed()) {
        startGame(&app->game, &app->cfg, MAP_FILES[app->cfg.selectedMap]);
        app->screen = SCREEN_GAME;
    }

    if (inputEditorPressed()) {
        freeMap(&app->editorMap);
        if (!loadMapFile(MAP_FILES[app->cfg.selectedMap], &app->editorMap)) {
            createBlankEditorMap(&app->editorMap);
        }
        app->editorCursor = (Pos){0, 0};
        snprintf(app->status, sizeof(app->status), "Editor abierto. S guarda en maps/custom_map.txt");
        app->screen = SCREEN_EDITOR;
    }
}

static void renderConfigScreen(const App *app)
{
    BeginDrawing();
    ClearBackground((Color){18, 20, 28, 255});

    DrawText("QUORIDOR PAC-MAN", 60, 42, 38, GOLD);
    DrawText("Configuracion de partida", 62, 88, 22, RAYWHITE);

    int y = 135;
    for (int i = 0; i < configItemCount(); i++) {
        Color c = (i == app->configCursor) ? YELLOW : RAYWHITE;
        DrawText(i == app->configCursor ? ">" : " ", 55, y, 22, c);
        DrawText(configItemText(&app->cfg, i), 85, y, 22, c);
        y += 32;
    }

    int helpX = 650;
    DrawText("Controles", helpX, 135, 24, SKYBLUE);
    DrawText("Flechas/D-Pad: elegir parametro", helpX, 175, 20, LIGHTGRAY);
    DrawText("Izq/der o D-Pad: cambiar valor", helpX, 205, 20, LIGHTGRAY);
    DrawText("Enter/X: iniciar partida", helpX, 235, 20, LIGHTGRAY);
    DrawText("E/R1: abrir editor de mapas", helpX, 265, 20, LIGHTGRAY);
    DrawText("ESC/Circulo: volver/salir", helpX, 295, 20, LIGHTGRAY);

    DrawText("Dificultad IA", helpX, 360, 24, SKYBLUE);
    DrawText("1: movimiento mas aleatorio", helpX, 400, 20, LIGHTGRAY);
    DrawText("2: persigue reduciendo distancia", helpX, 430, 20, LIGHTGRAY);
    DrawText("3: persigue y a veces usa muros", helpX, 460, 20, LIGHTGRAY);

    DrawText(app->status, 60, GetScreenHeight() - 65, 18, LIGHTGRAY);
    DrawText(activeGamepadName(), 60, GetScreenHeight() - 38, 18, SKYBLUE);
    EndDrawing();
}


// ============================================================
// SECCION 13: Pantalla de juego
// ============================================================

static void updateGameScreen(App *app)
{
    GameState *g = &app->game;

    if (inputBackPressed()) app->screen = SCREEN_CONFIG;
    if (inputRestartPressed()) restartMatch(g);

    if (g->winner != WIN_NONE) {
        app->screen = SCREEN_GAMEOVER;
        return;
    }

    if (g->currentTeam == TEAM_GHOSTS && g->cfg.mode == MODE_AI) {
        updateAI(g);
        return;
    }

    int dr = 0, dc = 0;
    bool arrow = readDirectionPressed(&dr, &dc);

    if (inputWallPressed()) {
        g->wallMode = !g->wallMode;
        snprintf(g->message, sizeof(g->message), g->wallMode ? "Modo muro: pulse una flecha para colocar." : "Modo muro cancelado.");
    }

    if (g->currentTeam == TEAM_PAC) {
        if (inputConfirmPressed()) endPacTurn(g);
        if (arrow) {
            if (g->wallMode) {
                Pos to = {g->pac.r + dr, g->pac.c + dc};
                if (tryPlaceTempWall(g, g->pac, to, OWNER_PAC)) {
                    g->wallMode = false;
                    if (g->actionsLeft <= 0) snprintf(g->message, sizeof(g->message), "Sin acciones. Enter para terminar turno.");
                }
            } else {
                tryMovePac(g, dr, dc);
            }
        }
    } else if (g->currentTeam == TEAM_GHOSTS && g->cfg.mode == MODE_PVP) {
        if (inputConfirmPressed()) finishGhostTurn(g);
        if (arrow && g->currentGhost >= 0) {
            if (g->wallMode) {
                Pos from = g->ghost[g->currentGhost];
                Pos to = {from.r + dr, from.c + dc};
                if (tryPlaceTempWall(g, from, to, OWNER_GHOSTS)) {
                    finishGhostTurn(g);
                }
            } else {
                tryMoveGhost(g, g->currentGhost, dr, dc);
                finishGhostTurn(g);
            }
        }
    }
}

static void renderGameScreen(const App *app)
{
    const GameState *g = &app->game;
    BeginDrawing();
    ClearBackground((Color){14, 18, 28, 255});

    int sideW = 320;
    int cell = computeCellSize(g->map.rows, g->map.cols, GetScreenWidth() - sideW - 80, GetScreenHeight() - 110);
    int boardW = g->map.cols * cell;
    int boardH = g->map.rows * cell;
    int ox = 45;
    int oy = 75;

    DrawText("Quoridor Pac-Man", 45, 25, 28, GOLD);
    DrawText(g->map.name, 300, 31, 18, LIGHTGRAY);
    renderBoardGame(g, ox, oy, cell);

    DrawRectangleLines(ox - 2, oy - 2, boardW + 4, boardH + 4, BLUE);
    drawHud(g, ox + boardW + 40, oy);

    DrawText(g->message, 45, GetScreenHeight() - 62, 19, RAYWHITE);
    DrawText(activeGamepadName(), 45, GetScreenHeight() - 35, 18, SKYBLUE);
    EndDrawing();
}


// ============================================================
// SECCION 14: Dibujo del tablero y HUD
// ============================================================

static int computeCellSize(int rows, int cols, int availableW, int availableH)
{
    int cw = availableW / cols;
    int ch = availableH / rows;
    int cell = (cw < ch) ? cw : ch;
    return clampInt(cell, 24, 58);
}

static void renderBoardGame(const GameState *g, int ox, int oy, int cell)
{
    for (int r = 0; r < g->map.rows; r++) {
        for (int c = 0; c < g->map.cols; c++) {
            Color tile = ((r + c) % 2 == 0) ? (Color){26, 34, 58, 255} : (Color){20, 27, 47, 255};
            DrawRectangle(ox + c * cell, oy + r * cell, cell, cell, tile);
            DrawRectangleLines(ox + c * cell, oy + r * cell, cell, cell, (Color){72, 86, 125, 255});
        }
    }

    for (int i = 0; i < g->map.wallCount; i++) {
        drawWallLine(g->map.walls[i], ox, oy, cell, (Color){150, 150, 160, 255}, 8, false);
    }
    for (int i = 0; i < g->tempCount; i++) {
        Color c = g->tempWalls[i].owner == OWNER_PAC ? (Color){255, 220, 80, 255} : (Color){120, 210, 255, 255};
        drawWallLine(g->tempWalls[i], ox, oy, cell, c, 5, true);
    }

    for (int i = 0; i < MAX_PELLETS; i++) {
        if (!g->pelletEaten[i]) {
            Pos p = g->map.pelletStart[i];
            DrawCircle(ox + p.c * cell + cell / 2, oy + p.r * cell + cell / 2, cell / 8, YELLOW);
        }
    }

    for (int i = 0; i < MAX_GHOSTS; i++) {
        if (g->cfg.ghostEnabled[i] && g->ghostAlive[i]) {
            Pos p = g->ghost[i];
            int x = ox + p.c * cell + cell / 2;
            int y = oy + p.r * cell + cell / 2;
            DrawCircle(x, y, cell / 3, GHOST_COLORS[i]);
            DrawCircle(x - cell / 10, y - cell / 10, cell / 12, WHITE);
            DrawCircle(x + cell / 10, y - cell / 10, cell / 12, WHITE);
            DrawCircle(x - cell / 10, y - cell / 10, cell / 24, BLACK);
            DrawCircle(x + cell / 10, y - cell / 10, cell / 24, BLACK);
            if (g->currentTeam == TEAM_GHOSTS && g->currentGhost == i) {
                DrawCircleLines(x, y, cell / 2 - 2, WHITE);
            }
        }
    }

    int px = ox + g->pac.c * cell + cell / 2;
    int py = oy + g->pac.r * cell + cell / 2;
    DrawCircle(px, py, cell / 3, GOLD);
    DrawCircle(px + cell / 9, py - cell / 9, cell / 14, BLACK);
    if (g->pacPoweredThisTurn) DrawCircleLines(px, py, cell / 2 - 2, GREEN);
}

static void drawWallLine(Wall w, int ox, int oy, int cell, Color color, int thickness, bool drawLife)
{
    int x1 = ox + w.a.c * cell;
    int y1 = oy + w.a.r * cell;
    int x2 = ox + w.b.c * cell;
    int y2 = oy + w.b.r * cell;

    if (w.a.r == w.b.r) {
        int cmin = (w.a.c < w.b.c) ? w.a.c : w.b.c;
        int x = ox + (cmin + 1) * cell;
        int y = oy + w.a.r * cell;
        DrawRectangle(x - thickness / 2, y + 3, thickness, cell - 6, color);
        if (drawLife) DrawText(TextFormat("%d", w.life), x + 5, y + cell / 2 - 8, 14, WHITE);
    } else if (w.a.c == w.b.c) {
        int rmin = (w.a.r < w.b.r) ? w.a.r : w.b.r;
        int x = ox + w.a.c * cell;
        int y = oy + (rmin + 1) * cell;
        DrawRectangle(x + 3, y - thickness / 2, cell - 6, thickness, color);
        if (drawLife) DrawText(TextFormat("%d", w.life), x + cell / 2 - 4, y + 5, 14, WHITE);
    } else {
        DrawLine(x1, y1, x2, y2, color);
    }
}

static void drawHud(const GameState *g, int x, int y)
{
    DrawText("Informacion", x, y, 25, SKYBLUE);
    y += 40;
    DrawText(TextFormat("Vidas: %d", g->lives), x, y, 20, RAYWHITE); y += 28;
    DrawText(TextFormat("Pac-bolas: %d/4", g->pelletsEaten), x, y, 20, RAYWHITE); y += 28;
    DrawText(TextFormat("Muros P-M: %d", g->pacWalls), x, y, 20, RAYWHITE); y += 28;
    DrawText(TextFormat("Muros Fant.: %d", g->ghostWalls), x, y, 20, RAYWHITE); y += 28;
    DrawText(TextFormat("Turno global: %d", g->globalTurn), x, y, 20, RAYWHITE); y += 35;

    if (g->currentTeam == TEAM_PAC) {
        DrawText("Turno: Pac-Man", x, y, 20, YELLOW); y += 28;
        DrawText(TextFormat("Acciones: %d", g->actionsLeft), x, y, 20, YELLOW); y += 28;
    } else {
        DrawText(TextFormat("Turno: %s", GHOST_NAMES[g->currentGhost]), x, y, 20, GHOST_COLORS[g->currentGhost]); y += 28;
        DrawText(TextFormat("Frenetico: %s", ghostCanSeePac(g, g->currentGhost) ? "Si" : "No"), x, y, 20, RAYWHITE); y += 28;
    }

    y += 15;
    DrawText("Controles", x, y, 22, SKYBLUE); y += 34;
    DrawText("Flechas/D-Pad: avanzar", x, y, 18, LIGHTGRAY); y += 25;
    DrawText("M/Triangulo: colocar muro", x, y, 18, LIGHTGRAY); y += 25;
    DrawText("Enter/X/Options: fin turno", x, y, 18, LIGHTGRAY); y += 25;
    DrawText("R/Cuadrado: reiniciar", x, y, 18, LIGHTGRAY); y += 25;
    DrawText("ESC/Circulo: menu", x, y, 18, LIGHTGRAY); y += 25;

    if (g->wallMode) {
        y += 15;
        DrawText("MODO MURO", x, y, 22, ORANGE); y += 26;
        DrawText("Pulsa una flecha", x, y, 18, ORANGE);
    }
}


// ============================================================
// SECCION 15: Inteligencia artificial de fantasmas
// ============================================================

static void updateAI(GameState *g)
{
    if (g->currentGhost < 0 || g->winner != WIN_NONE) return;
    if (GetTime() - g->aiTimer < 0.35) return;
    g->aiTimer = GetTime();

    int ghost = g->currentGhost;
    bool placed = aiTryPlaceWall(g, ghost);
    if (!placed) aiMoveGhost(g, ghost);
    finishGhostTurn(g);
}

static bool aiTryPlaceWall(GameState *g, int ghostIndex)
{
    int diff = g->cfg.ghostDifficulty[ghostIndex];
    if (diff < 3 || g->ghostWalls <= 0) return false;
    if (GetRandomValue(1, 100) > 25) return false;

    Pos origin = g->ghost[ghostIndex];
    Pos dirs[4] = {{-1,0},{1,0},{0,-1},{0,1}};
    int best = -1;
    int bestScore = 9999;

    for (int i = 0; i < 4; i++) {
        Pos to = {origin.r + dirs[i].r, origin.c + dirs[i].c};
        if (!insideMap(&g->map, to)) continue;
        if (findWall(g->map.walls, g->map.wallCount, origin, to) >= 0) continue;
        if (findWall(g->tempWalls, g->tempCount, origin, to) >= 0) continue;

        int dist = abs(to.r - g->pac.r) + abs(to.c - g->pac.c);
        if (dist < bestScore) {
            bestScore = dist;
            best = i;
        }
    }

    if (best >= 0) {
        Pos to = {origin.r + dirs[best].r, origin.c + dirs[best].c};
        return tryPlaceTempWall(g, origin, to, OWNER_GHOSTS);
    }
    return false;
}

static void aiMoveGhost(GameState *g, int ghostIndex)
{
    Pos dirs[4] = {{-1,0},{1,0},{0,-1},{0,1}};
    int order[4] = {0, 1, 2, 3};
    int diff = g->cfg.ghostDifficulty[ghostIndex];

    if (diff == 1 && GetRandomValue(1, 100) <= 45) {
        for (int i = 0; i < 4; i++) {
            int j = GetRandomValue(0, 3);
            int tmp = order[i];
            order[i] = order[j];
            order[j] = tmp;
        }
    } else {
        int scores[4];
        for (int i = 0; i < 4; i++) {
            Pos sim = g->ghost[ghostIndex];
            int steps = moveStepsForGhost(g, ghostIndex);
            for (int s = 0; s < steps; s++) {
                Pos next = {sim.r + dirs[i].r, sim.c + dirs[i].c};
                if (isBlocked(g, sim, next)) break;
                sim = next;
            }
            scores[i] = abs(sim.r - g->pac.r) + abs(sim.c - g->pac.c);
        }
        for (int i = 0; i < 4; i++) order[i] = i;
        for (int i = 0; i < 4; i++) {
            for (int j = i + 1; j < 4; j++) {
                if (scores[order[j]] < scores[order[i]]) {
                    int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
                }
            }
        }
    }

    for (int k = 0; k < 4; k++) {
        int i = order[k];
        Pos next = {g->ghost[ghostIndex].r + dirs[i].r, g->ghost[ghostIndex].c + dirs[i].c};
        if (!isBlocked(g, g->ghost[ghostIndex], next)) {
            tryMoveGhost(g, ghostIndex, dirs[i].r, dirs[i].c);
            return;
        }
    }

    snprintf(g->message, sizeof(g->message), "%s no tiene movimientos legales.", GHOST_NAMES[ghostIndex]);
}


// ============================================================
// SECCION 16: Editor de mapas
// ============================================================

static void updateEditorScreen(App *app)
{
    MapData *m = &app->editorMap;

    if (inputBackPressed() || inputConfirmPressed()) {
        app->screen = SCREEN_CONFIG;
        snprintf(app->status, sizeof(app->status), "Volviste al menu. Mapa editor: %s", MAP_FILES[3]);
        return;
    }

    if (inputUpPressed()) app->editorCursor.r = clampInt(app->editorCursor.r - 1, 0, m->rows - 1);
    if (inputDownPressed()) app->editorCursor.r = clampInt(app->editorCursor.r + 1, 0, m->rows - 1);
    if (inputLeftPressed()) app->editorCursor.c = clampInt(app->editorCursor.c - 1, 0, m->cols - 1);
    if (inputRightPressed()) app->editorCursor.c = clampInt(app->editorCursor.c + 1, 0, m->cols - 1);

    if (IsKeyPressed(KEY_EQUAL)) { m->rows = clampInt(m->rows + 1, MIN_BOARD, MAX_BOARD); cleanMapAfterResize(m); }
    if (IsKeyPressed(KEY_MINUS)) { m->rows = clampInt(m->rows - 1, MIN_BOARD, MAX_BOARD); cleanMapAfterResize(m); app->editorCursor.r = clampInt(app->editorCursor.r, 0, m->rows - 1); }
    if (IsKeyPressed(KEY_PERIOD)) { m->cols = clampInt(m->cols + 1, MIN_BOARD, MAX_BOARD); cleanMapAfterResize(m); }
    if (IsKeyPressed(KEY_COMMA)) { m->cols = clampInt(m->cols - 1, MIN_BOARD, MAX_BOARD); cleanMapAfterResize(m); app->editorCursor.c = clampInt(app->editorCursor.c, 0, m->cols - 1); }

    if (IsKeyPressed(KEY_P)) m->pacStart = app->editorCursor;
    if (IsKeyPressed(KEY_ONE)) m->ghostStart[0] = app->editorCursor;
    if (IsKeyPressed(KEY_TWO)) m->ghostStart[1] = app->editorCursor;
    if (IsKeyPressed(KEY_THREE)) m->ghostStart[2] = app->editorCursor;
    if (IsKeyPressed(KEY_FOUR)) m->ghostStart[3] = app->editorCursor;
    if (IsKeyPressed(KEY_Q)) m->pelletStart[0] = app->editorCursor;
    if (IsKeyPressed(KEY_W)) m->pelletStart[1] = app->editorCursor;
    if (IsKeyPressed(KEY_E)) m->pelletStart[2] = app->editorCursor;
    if (IsKeyPressed(KEY_R)) m->pelletStart[3] = app->editorCursor;

    if (IsKeyPressed(KEY_H)) {
        Pos a = app->editorCursor;
        Pos b = {a.r, a.c + 1};
        if (insideMap(m, b)) {
            int idx = findWall(m->walls, m->wallCount, a, b);
            if (idx >= 0) removeWallAt(m->walls, &m->wallCount, idx);
            else addWallDynamic(&m->walls, &m->wallCount, &m->wallCap, makeWall(a, b, OWNER_PERMANENT, -1));
        }
    }
    if (IsKeyPressed(KEY_V)) {
        Pos a = app->editorCursor;
        Pos b = {a.r + 1, a.c};
        if (insideMap(m, b)) {
            int idx = findWall(m->walls, m->wallCount, a, b);
            if (idx >= 0) removeWallAt(m->walls, &m->wallCount, idx);
            else addWallDynamic(&m->walls, &m->wallCount, &m->wallCap, makeWall(a, b, OWNER_PERMANENT, -1));
        }
    }

    if (inputSavePressed()) {
        strncpy(m->name, "Mapa del editor", sizeof(m->name) - 1);
        if (saveMapFile(MAP_FILES[3], m)) {
            app->cfg.selectedMap = 3;
            snprintf(app->status, sizeof(app->status), "Mapa guardado en %s", MAP_FILES[3]);
        } else {
            snprintf(app->status, sizeof(app->status), "No se pudo guardar el mapa.");
        }
    }
}

static void renderEditorScreen(const App *app)
{
    const MapData *m = &app->editorMap;
    BeginDrawing();
    ClearBackground((Color){16, 18, 26, 255});

    int sideW = 360;
    int cell = computeCellSize(m->rows, m->cols, GetScreenWidth() - sideW - 80, GetScreenHeight() - 110);
    int ox = 45;
    int oy = 75;

    DrawText("Editor de mapas", 45, 25, 30, GOLD);
    renderBoardEditor(m, app->editorCursor, ox, oy, cell);

    int x = ox + m->cols * cell + 40;
    int y = oy;
    DrawText("Controles", x, y, 24, SKYBLUE); y += 38;
    DrawText("Flechas/D-Pad: mover cursor", x, y, 18, LIGHTGRAY); y += 25;
    DrawText("+ / - : filas", x, y, 18, LIGHTGRAY); y += 25;
    DrawText(", / . : columnas", x, y, 18, LIGHTGRAY); y += 25;
    DrawText("P: Pac-Man", x, y, 18, LIGHTGRAY); y += 25;
    DrawText("1 2 3 4: fantasmas", x, y, 18, LIGHTGRAY); y += 25;
    DrawText("Q W E R: pac-bolas", x, y, 18, LIGHTGRAY); y += 25;
    DrawText("H: muro derecha", x, y, 18, LIGHTGRAY); y += 25;
    DrawText("V: muro abajo", x, y, 18, LIGHTGRAY); y += 25;
    DrawText("S/L1: guardar custom_map", x, y, 18, LIGHTGRAY); y += 25;
    DrawText("Enter/X o ESC/Circulo: volver", x, y, 18, LIGHTGRAY); y += 40;
    DrawText(TextFormat("Tamano: %d x %d", m->rows, m->cols), x, y, 20, YELLOW); y += 28;
    DrawText(TextFormat("Muros permanentes: %d", m->wallCount), x, y, 20, YELLOW);

    DrawText(app->status, 45, GetScreenHeight() - 65, 18, RAYWHITE);
    DrawText(activeGamepadName(), 45, GetScreenHeight() - 38, 18, SKYBLUE);
    EndDrawing();
}

static void renderBoardEditor(const MapData *m, Pos cursor, int ox, int oy, int cell)
{
    for (int r = 0; r < m->rows; r++) {
        for (int c = 0; c < m->cols; c++) {
            Color tile = ((r + c) % 2 == 0) ? (Color){28, 34, 56, 255} : (Color){22, 27, 45, 255};
            DrawRectangle(ox + c * cell, oy + r * cell, cell, cell, tile);
            DrawRectangleLines(ox + c * cell, oy + r * cell, cell, cell, (Color){74, 88, 122, 255});
        }
    }

    for (int i = 0; i < m->wallCount; i++) {
        drawWallLine(m->walls[i], ox, oy, cell, (Color){175, 175, 185, 255}, 8, false);
    }

    for (int i = 0; i < MAX_PELLETS; i++) {
        Pos p = m->pelletStart[i];
        DrawCircle(ox + p.c * cell + cell / 2, oy + p.r * cell + cell / 2, cell / 8, YELLOW);
        DrawText(TextFormat("%d", i + 1), ox + p.c * cell + 4, oy + p.r * cell + 4, 12, WHITE);
    }

    for (int i = 0; i < MAX_GHOSTS; i++) {
        Pos p = m->ghostStart[i];
        DrawCircle(ox + p.c * cell + cell / 2, oy + p.r * cell + cell / 2, cell / 3, GHOST_COLORS[i]);
        DrawText(TextFormat("%d", i + 1), ox + p.c * cell + cell / 2 - 4, oy + p.r * cell + cell / 2 - 7, 14, BLACK);
    }

    Pos p = m->pacStart;
    DrawCircle(ox + p.c * cell + cell / 2, oy + p.r * cell + cell / 2, cell / 3, GOLD);
    DrawText("P", ox + p.c * cell + cell / 2 - 5, oy + p.r * cell + cell / 2 - 8, 16, BLACK);

    DrawRectangleLinesEx((Rectangle){ox + cursor.c * cell + 2, oy + cursor.r * cell + 2, cell - 4, cell - 4}, 3, GREEN);
}


// ============================================================
// SECCION 17: Pantalla final
// ============================================================

static void renderGameOverScreen(const App *app)
{
    const GameState *g = &app->game;
    BeginDrawing();
    ClearBackground((Color){12, 14, 22, 255});

    const char *winner = (g->winner == WIN_PAC) ? "Gana Pac-Man" : "Ganan los fantasmas";
    DrawText("Fin de la partida", 80, 80, 42, GOLD);
    DrawText(winner, 80, 150, 34, RAYWHITE);
    DrawText(TextFormat("Pac-bolas comidas: %d/4", g->pelletsEaten), 80, 210, 26, YELLOW);
    DrawText(TextFormat("Nivel alcanzado: %s", levelName(g->pelletsEaten)), 80, 250, 26, SKYBLUE);

    DrawText("R/Cuadrado: reiniciar la partida", 80, 340, 22, LIGHTGRAY);
    DrawText("Enter/X/Circulo: volver al menu", 80, 375, 22, LIGHTGRAY);
    DrawText("ESC: salir", 80, 410, 22, LIGHTGRAY);
    DrawText(activeGamepadName(), 80, 455, 18, SKYBLUE);

    EndDrawing();
}


// ============================================================
// SECCION 18: Funcion principal
// ============================================================

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_W, SCREEN_H, "Quoridor Pac-Man - LP1");
    initGameAudio();
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)time(NULL));

    GHOST_COLORS[0] = RED;
    GHOST_COLORS[1] = SKYBLUE;
    GHOST_COLORS[2] = PINK;
    GHOST_COLORS[3] = ORANGE;

    ensureDefaultMaps();

    App app;
    memset(&app, 0, sizeof(App));
    app.screen = SCREEN_CONFIG;
    initDefaultConfig(&app.cfg);
    initMap(&app.editorMap, DEFAULT_ROWS, DEFAULT_COLS, "Mapa creado");
    app.editorCursor = (Pos){0, 0};
    snprintf(app.status, sizeof(app.status), "Listo. Enter inicia, E abre el editor.");

    while (!WindowShouldClose()) {
        updateGameAudio();

        switch (app.screen) {
            case SCREEN_CONFIG:
                updateConfigScreen(&app);
                renderConfigScreen(&app);
                break;
            case SCREEN_GAME:
                updateGameScreen(&app);
                renderGameScreen(&app);
                break;
            case SCREEN_EDITOR:
                updateEditorScreen(&app);
                renderEditorScreen(&app);
                break;
            case SCREEN_GAMEOVER:
                if (inputRestartPressed()) {
                    restartMatch(&app.game);
                    app.screen = SCREEN_GAME;
                }
                if (inputConfirmPressed() || inputBackPressed()) app.screen = SCREEN_CONFIG;
                renderGameOverScreen(&app);
                break;
        }
    }

    freeGame(&app.game);
    freeMap(&app.editorMap);
    closeGameAudio();
    CloseWindow();
    return 0;
}
