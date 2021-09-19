cmake_minimum_required(VERSION 3.17)
project(alphac C)

set(CMAKE_C_STANDARD 11)


add_executable(syntactic_analyzer "syntactic_analyzer/libs/stack.c" "syntactic_analyzer/libs/structs.c" "syntactic_analyzer/libs/symlist.c" "syntactic_analyzer/libs/symtable.c" "syntactic_analyzer/scanner.c" "syntactic_analyzer/parser.c")

