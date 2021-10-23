#!/bin/sh
gcc -pipe -Wall -Wextra raw2mod.c -o raw2mod -lc -lm
chmod +x raw2mod
