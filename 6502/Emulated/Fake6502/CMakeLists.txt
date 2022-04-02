cmake_minimum_required(VERSION 3.0)

project(fake6502 LANGUAGES C)

add_executable(fake6502 main.c)

add_library(
   myne
   Myne/myne.h
   Myne/fake6502.c
)

target_link_libraries(fake6502
   myne
)



install(TARGETS fake6502 RUNTIME DESTINATION bin)
