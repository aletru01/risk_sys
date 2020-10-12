CXX = gcc
CXXFLAGS = -g -lstdc++ -std=gnu++17 -Wall -Wextra -Werror -pthread
VPATH = src

server: server.cc parse_msg.cc update_table.cc

clean:
	$(RM) server
