#!/bin/bash

# List of source files
files=("show_proto_info.c"
       "show_service_info.c"
       "show_addr_info.c"
       # Add more examples here
      )

# Compile each file into a separate executable
for src in "${files[@]}"; do
    exe="${src%.c}"
    gcc -Wall -Wextra -O0 "$src" -o "$exe"
done
