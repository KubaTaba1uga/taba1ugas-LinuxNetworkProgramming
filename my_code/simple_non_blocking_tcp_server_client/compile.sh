#!/bin/bash

# List of source files
files=("server.c"
       "client.c"
      )

# Compile each file into a separate executable
for src in "${files[@]}"; do
    exe="${src%.c}"
    clang -g -no-pie -fno-pie -fsanitize=address,undefined -O0 -Wall "$src" -o "$exe" -pthread
done
