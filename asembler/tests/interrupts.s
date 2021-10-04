# file interrupts.s
.section ivt
.word isr_reset
.skip 2 # isr_error
.word isr_timer
.word isr_terminal
.skip 8
.extern myStart,myCounter
.section isr
.equ term_out,0xFF00
.equ term_in,0xFF02
.equ asciiCode,84 # ascii(’T’)
# prekidna rutina za reset
isr_reset:
jmp myStart
# prekidna rutina za tajmer
isr_timer:
push r0
ldr r0,$asciiCode
str r0,term_out
pop r0
iret
# prekidna rutina za terminal
isr_terminal:
push r0
push r1
ldr r0,term_in
str r0,term_out
ldr r0,%myCounter # pcrel
ldr r1,$1
add r0,r1
str r0,myCounter # abs
pop r1
pop r0
iret
.end