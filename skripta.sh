./zadatak1/asembler -o ./zadatak1/tests/interrupts.o ./zadatak1/tests/interrupts.s
./zadatak1/asembler -o ./zadatak1/tests/main.o ./zadatak1/tests/main.s

./zadatak2/linker -hex -place=ivt@0x0000 -o ./zadatak2/tests/program.hex ./zadatak1/tests/interrupts.o ./zadatak1/tests/main.o

./zadatak3/emulator ./zadatak2/tests/program.hex