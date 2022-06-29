# Automated Testing Documentation

## To Create Test Environment Container

From the root of the repository:

    docker build --rm=false \
        -t wortheumio/ci-test-environment:latest \
        -f tests/scripts/Dockerfile.testenv .

## To Run The Tests

(Also in the root of the repository.)

    docker build --rm=false \
        -t wortheumio/wortheum-test \
        -f Dockerfile.test .

## To Troubleshoot Failing Tests

    docker run -ti \
        wortheumio/ci-test-environment:latest \
        /bin/bash

Then, inside the container:

(These steps are taken from `/Dockerfile.test` in the
repository root.)

    git clone https://github.com/wortheumio/wortheum.git \
        /usr/local/src/wortheum
    cd /usr/local/src/wortheum
    git checkout <branch> # e.g. 123-feature
    git submodule update --init --recursive
    mkdir build
    cd build
    cmake \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_WORTHEUM_TESTNET=ON \
        -DLOW_MEMORY_NODE=OFF \
        -DCLEAR_VOTES=ON \
        ..
    make -j$(nproc) chain_test
    ./tests/chain_test
    cd /usr/local/src/wortheum
    doxygen
    programs/build_helpers/check_reflect.py
