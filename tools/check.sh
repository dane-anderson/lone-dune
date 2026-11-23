#!/usr/bin/env bash
# Check the rover without opening a window. Keep the executable in build/.
set -e
cd "$(dirname "$0")/.."
mkdir -p build tests/results
FILES=(tests/tests.cpp)
for FILE in rover/*.cpp; do
    case "$(basename "$FILE")" in
        main.cpp|WorldView.cpp) ;;
        *) FILES+=("$FILE") ;;
    esac
done
c++ -std=c++17 -Wall -Wextra -I. "${FILES[@]}" -o build/rover-tests
cd tests
../build/rover-tests
