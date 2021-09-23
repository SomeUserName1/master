#!/bin/zsh

cd ../build
llvm-cov gcov -l -p src/**/*.gcno
mkdir -p coverage
cd ..
gcovr -g -k -r . --html --html-details -o build/coverage/report.html
cd build
rm **/*.(gcov|gcno|gcda)
