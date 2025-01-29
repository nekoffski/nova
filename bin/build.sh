#!/bin/bash

SRC=$(dirname "$0")/../
MODE=$1

git submodule update --init --recursive

if [ "$MODE" = 'dev' ]; then
    conan install ${SRC}/conan/ --output-folder=${SRC}/build --build=missing --profile ${SRC}/conan/profiles/debug
    cd ${SRC}/build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DSL_ENABLE_UNIT_TESTS=On \
        -DSL_ENABLE_COVERAGE=Off -DSL_ENABLE_BENCHMARKS=On -DSL_BUILD_TYPE=DEV
elif [ "$MODE" = 'debug' ]; then
    conan install ${SRC}/conan/ --output-folder=${SRC}/build --build=missing --profile ${SRC}/conan/profiles/debug
    cd ${SRC}/build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug -DSL_ENABLE_UNIT_TESTS=On \
        -DSL_ENABLE_COVERAGE=On -DSL_ENABLE_BENCHMARKS=On -DSL_BUILD_TYPE:STRING=DEBUG
elif [ "$MODE" = 'release' ]; then
    exit -2
else
    echo "Invalid build mode - '" $MODE "'"
    exit -1
fi

cmake --build . -j`nproc`
