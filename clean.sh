#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

set -x

rm $DIR/*.o
rm $DIR/*.so
rm $DIR/*.exe

set +x

