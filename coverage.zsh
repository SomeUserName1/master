#!/bin/zsh

cd /home/someusername/bwsync/workspace/uni/master/build
llvm-cov gcov -b -c -l -p src/**/*.gcno
mkdir -p coverage
cd ..
gcovr -g -k -r . --html --html-details -o build/coverage/report.html
cd build
rm *.gcov
