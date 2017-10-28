#!/bin/sh
find . -iname *.h -o -iname *.cpp -o -iname *.c -o -iname *.cc | xargs clang-format -i --style=file 
