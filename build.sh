#!/bin/bash

cd engine
make clean
make

cd ../interface
npm run build
cp ../ /build