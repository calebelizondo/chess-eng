#!/bin/bash

cd engine
make clean
make

cd ../interface
npm install
npm run build