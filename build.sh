#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

set -e
set -x

#so for preload

echo "fork_hack => fork_hack.so:"
clang++ --std=c++1z -O0 -g -I"/usr/include" -fPIC -shared -o $DIR/fork_hack.so $DIR/fork_hack.cpp

#watcher

echo "fork_hack_watcher compile:"
clang++ --std=c++1z -Os -I"/usr/include" -c -o $DIR/fork_hack_watcher.o $DIR/fork_hack_watcher.cpp

echo "fork_hack_watcher link:"
clang++ --std=c++1z -Os $DIR/fork_hack_watcher.o -o $DIR/fork_hack_watcher.exe -L"/usr/lib/x86_64-linux-gnu" -lstdc++fs -lpthread

set +x
set +e

