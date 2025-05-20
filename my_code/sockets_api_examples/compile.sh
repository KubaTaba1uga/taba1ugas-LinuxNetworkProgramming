#!/bin/bash

# List of source files
files=("show_proto_info.c"
       "show_service_info.c"
       "show_addr_info.c"
       "convert_byte_order_between_host_and_network.c"
       "create_socket.c"
       "set_socket_options.c"
       "bind_socket_to_address.c"
       "listsen_on_socket.c"
       "connect_to_socket.c"
       # Add more examples here
      )

# Compile each file into a separate executable
for src in "${files[@]}"; do
    exe="${src%.c}"
    gcc -Wall -Wextra -O0 "$src" -o "$exe"
done
