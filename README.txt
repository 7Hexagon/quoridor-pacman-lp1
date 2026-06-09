# Quoridor Pac-Man - Trabajo Practico LP1

Implementacion en C usando raylib de una variante jugable de Quoridor Pac-Man.

El juego permite configurar la partida, cargar mapas predefinidos, crear mapas desde un editor propio, jugar en modo IA vs Player o Player vs Player, usar muros temporales, manejar fantasmas con dificultad configurable y utilizar teclado o control/gamepad.

## Estructura del proyecto

```text
C/
├─ assets/
│  ├─ music/
│  │  └─ background.wav
│  └─ sounds/
│     ├─ eat.wav
│     └─ death.wav
├─ maps/
│  ├─ mapa_clasico_9x9.map
│  ├─ mapa_pasillos_7x11.map
│  ├─ mapa_lab_11x9.map
│  └─ custom_map.txt
├─ src/
│  └─ TrabajoPractico1/
│     └─ main1.c
└─ README.md
```

## Dependencias

En Ubuntu, Linux Mint o Debian:

```bash
sudo apt update
sudo apt install -y build-essential git make cmake pkg-config
sudo apt install -y libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

## Instalacion de raylib

Si raylib no esta instalada en el sistema, se puede instalar desde el codigo fuente:

```bash
git clone --depth 1 https://github.com/raysan5/raylib.git
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP
sudo make install
sudo ldconfig
```

Si raylib ya esta instalada, este paso se puede omitir.

## Compilacion y ejecucion

Desde la raiz del proyecto, es decir, desde la carpeta `C/`:

```bash
gcc src/TrabajoPractico1/main1.c -o quoridor_pacman -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

Luego ejecutar:

```bash
./quoridor_pacman
```

Tambien se puede compilar y ejecutar en un solo comando:

```bash
gcc src/TrabajoPractico1/main1.c -o quoridor_pacman -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 && ./quoridor_pacman
```

Es importante ejecutar el programa desde la raiz del proyecto para que encuentre correctamente las carpetas `maps/` y `assets/`.

## Archivos de sonido

El programa tiene soporte opcional para sonido.

Rutas usadas:

```text
assets/music/background.wav
assets/sounds/eat.wav
assets/sounds/death.wav
```

Si estos archivos existen, el juego reproduce musica de fondo y efectos de sonido.

Si los archivos no existen, el juego sigue funcionando normalmente sin sonido.

## Como jugar

El juego tiene dos modos principales:

```text
IA vs Player
Player vs Player
```

En IA vs Player, Pac-Man es manejado por el jugador y los fantasmas son manejados por la computadora.

En Player vs Player, un jugador controla a Pac-Man y otro jugador controla a los fantasmas.

## Reglas principales

Pac-Man tiene 3 vidas.

Pac-Man gana si come las 4 pac-bolas.

Los fantasmas ganan si atrapan a Pac-Man 3 veces.

Pac-Man tiene 2 acciones por turno. Cada accion puede ser moverse una casilla o colocar un muro temporal.

Si Pac-Man come una pac-bola, gana una accion extra en ese turno.

Cada fantasma puede moverse una casilla o colocar un muro temporal.

Si un fantasma ve a Pac-Man en linea recta sin muros en el medio, entra en modo frenetico y puede moverse 2 casillas.

Los muros permanentes vienen definidos por el mapa.

Los muros temporales son colocados durante la partida, duran una cantidad configurable de turnos y luego vuelven a la mano del equipo que los coloco.

## Dificultad de los fantasmas

Cada fantasma tiene una dificultad independiente.

```text
Dificultad 1: movimiento mas aleatorio.
Dificultad 2: intenta perseguir a Pac-Man reduciendo la distancia.
Dificultad 3: persigue a Pac-Man y tambien puede usar muros temporales.
```

La dificultad solo afecta al modo IA vs Player. En Player vs Player, los fantasmas son controlados por un jugador humano.

## Controles de menu

```text
Flechas arriba/abajo o D-Pad: elegir opcion
Flechas izquierda/derecha o D-Pad: cambiar valor
Enter, X/Cross u Options/Start: iniciar partida
E o R1: abrir editor de mapas
ESC, Circle o Select/Create: volver o salir
```

## Controles de juego

```text
Flechas o D-Pad: mover ficha
M o Triangle: activar/desactivar modo muro
Enter, X/Cross u Options/Start: terminar turno
R o Square: reiniciar partida
ESC o Circle: volver al menu
```

## Controles del editor de mapas

```text
Flechas o D-Pad: mover cursor
+ / - : cambiar cantidad de filas
, / . : cambiar cantidad de columnas
P: colocar Pac-Man
1, 2, 3, 4: colocar fantasmas
Q, W, E, R: colocar pac-bolas
H: colocar o quitar muro hacia la derecha
V: colocar o quitar muro hacia abajo
S o L1: guardar mapa personalizado
Enter, X/Cross, ESC o Circle: volver al menu
```

El mapa creado desde el editor se guarda en:

```text
maps/custom_map.txt
```

Luego puede elegirse desde la pantalla de configuracion como `Mapa del editor`.

## Formato de los mapas

Los mapas son archivos de texto.

Ejemplo:

```text
NAME Clasico 9x9
SIZE 9 9
PAC 8 4
GHOST 0 0 4
GHOST 1 0 0
GHOST 2 0 8
GHOST 3 4 4
PELLET 0 1 1
PELLET 1 1 7
PELLET 2 7 1
PELLET 3 7 7
WALL 2 1 2 2
END
```

Significado:

```text
NAME: nombre del mapa
SIZE: cantidad de filas y columnas
PAC: posicion inicial de Pac-Man
GHOST: posicion inicial de cada fantasma
PELLET: posicion de cada pac-bola
WALL: muro permanente entre dos casillas
END: fin del archivo
```

## Reinicio y fin de partida

Durante la partida se puede presionar `R` o `Square` para reiniciar.

Al terminar la partida, se muestra el ganador y el nivel alcanzado por Pac-Man segun la cantidad de pac-bolas comidas:

```text
1: Pac-Man novato
2: Pac-Man prometedor
3: Pac-Man de categoria
4: Pac-Man de elite
```

## Notas

El proyecto usa un solo archivo principal, `main1.c`, para facilitar la compilacion en la computadora del docente.

Los archivos de sonido son opcionales.

Los mapas predefinidos se guardan en la carpeta `maps/`.

El programa debe ejecutarse desde la raiz del proyecto para que las rutas relativas funcionen correctamente.

