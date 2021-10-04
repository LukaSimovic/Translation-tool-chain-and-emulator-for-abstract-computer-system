./asembler/asembler -o ./asembler/tests/interrupts.o ./asembler/tests/interrupts.s
./asembler/asembler -o ./asembler/tests/main.o ./asembler/tests/main.s

./linker/linker -hex -place=ivt@0x0000 -o ./linker/tests/program.hex ./asembler/tests/interrupts.o ./asembler/tests/main.o

./emulator/emulator ./linker/tests/program.hex
