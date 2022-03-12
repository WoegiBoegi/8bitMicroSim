#!/bin/bash
gcc CPUSim.c BIOSSim.c -ICPUSim -IBIOSSim -lncurses -o microsim
