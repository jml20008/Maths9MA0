#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
cc -std=c11 -O2 -Wall -Wextra -Werror -pedantic src/engine.c tests/test_engine.c -lm -o tests/test_engine
./tests/test_engine
cc -Itests/stubs -Isrc -std=c11 -Wall -Wextra -Werror -pedantic -Dmain=app_main -c src/main.c -o tests/mock_main.o
cc -Itests/stubs -Isrc -std=c11 -Wall -Wextra -Werror -pedantic tests/test_ui.c tests/mock_main.o src/app_data.c src/engine.c -lm -o tests/test_ui
./tests/test_ui
rm -f tests/mock_main.o
