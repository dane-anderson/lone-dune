#!/usr/bin/env bash
# Build the rover and keep compiled files in build/.
# Run: bash tools/build.sh
# Build without opening the window: bash tools/build.sh --build-only
set -e
cd "$(dirname "$0")/.."
mkdir -p build
case "$(uname -s)" in
    Darwin)
        C_COMPILER=clang
        CPP_COMPILER=clang++
        C_FLAGS=(-DOBJC_OLD_DISPATCH_PROTOTYPES=1 -Wno-deprecated-declarations)
        LIBRARIES=(-framework OpenGL -framework Cocoa)
        PROGRAM=lone-dune ;;
    Linux)
        C_COMPILER=gcc
        CPP_COMPILER=g++
        C_FLAGS=()
        LIBRARIES=(-lGLU -lGL -lX11)
        PROGRAM=lone-dune ;;
    MINGW*|MSYS*|CYGWIN*)
        C_COMPILER=gcc
        CPP_COMPILER=g++
        C_FLAGS=()
        LIBRARIES=(-static -lopengl32 -lgdi32)
        PROGRAM=lone-dune.exe ;;
    *) echo "This system is not supported by this build script."; exit 1 ;;
esac
"$C_COMPILER" -x c -c library/tigr.c -o build/tigr.o "${C_FLAGS[@]}"
"$CPP_COMPILER" -std=c++17 -Wall -Wextra -I. -Ilibrary rover/*.cpp library/graphics/*.cpp build/tigr.o "${LIBRARIES[@]}" -o "build/$PROGRAM"
echo "Lone Dune is built."
if [ "${1:-}" != "--build-only" ]; then
    cd rover
    exec "../build/$PROGRAM"
fi
