global start

BITS 64
org 0x4000000

start:

mov ax, 0x10
mov ss, ax
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
sub rsp, 0x10
mov qword [rsp + 0x8], 0x8
mov qword [rsp], target
db 0x48
retf
target:

vmcall
hlt
