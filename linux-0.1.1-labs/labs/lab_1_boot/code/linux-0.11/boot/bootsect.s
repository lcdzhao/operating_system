SETUPLEN=2
SETUPSEG=0x07e0
entry _start
_start:
    ! read cursor pos , pos will be in dx 
    mov ah,#0x03 
    xor bh,bh
    int 0x10

    ! show msg1
    ! cx is the length of msg1
    mov cx,#38

    ! es:bp is the address of msg1
    mov bp,#msg1
    mov ax,#0x07c0
    mov es,ax

    ! page 0, attribute7 (normal)
    mov bx,#0x0007

    ! write string, move cursor
    mov ax,#0x1301
    int 0x10

load_setup:
    !set driver(0) and header(0)
    mov dx,#0x0000

    !set track(0) and sector(2)
    mov cx,#0x0002

    !set load to where()
    mov bx,#0x0200
    mov ax,#0x0200+SETUPLEN

    ! use int 0x13 to load from disk
    int 0x13
    
    !ok_load_setup: ok - continue
    jnc ok_load_setup

    !reset the diskette
    mov dx,#0x0000
    mov ax,#0x0000
    int 0x13

    !load again
    jmp load_setup

ok_load_setup:
    !jump to setup
    jmpi 0,  SETUPSEG

msg1:
    .byte   13,10
    .ascii  "Hello world!I,m WithoutOS!   ^_< !  "
    .byte   13,10,13,10

.org 510 		! org is used to specific physic address
boot_flag:
    .word   0xAA55
