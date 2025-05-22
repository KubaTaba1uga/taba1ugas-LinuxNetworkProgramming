#!/bin/bash

# List of source files
files=("pong_server.c"
       "ping_client.c"
      )

# Compile each file into a separate executable
for src in "${files[@]}"; do
    exe="${src%.c}"
    gcc -g -Wall -Wextra -O0 "$src" -o "$exe" -pthread
done
