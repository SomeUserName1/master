#!/bin/bash

cd ../build
llvm-cov gcov -a -b -l -p $(find src -name "*.gcno" -type f)
mkdir -p coverage
cd ..
gcovr -g -k -r . --html --html-details -o build/coverage/report.html
cd build
codecov
find . -name "*.gcov" -type f -exec rm \{\} \;
find . -name "*.gcno" -type f -exec rm \{\} \;
find . -name "*.gcda" -type f -exec rm \{\} \;

