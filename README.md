# CHIP-8 Emulator

Un sencillo emulador de **CHIP-8** escrito en C y utilizando **SDL2** para gráficos y entrada.

## Dependencias

- Compilador C compatible con C99 (`gcc` o `clang`)
- [SDL2](https://www.libsdl.org/)

---

## Compilación en Linux

### 1. Instalar dependencias
En distribuciones basadas en Debian/Ubuntu:

```bash
sudo apt update
sudo apt install build-essential libsdl2-dev
```

### 2. Usar makefile
```bash
make
./main
```

## Compilación en Windows

### 1. Instalar MSYS2

### 2. Instalar dependencias

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 make
```

### 3. Usar makefile
```bash
make
./main
```
