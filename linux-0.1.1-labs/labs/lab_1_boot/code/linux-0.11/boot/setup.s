INITSEG  = 0x9000
entry _start
_start:
    ! Read cursor pos , pos will be store in dx 
    mov ah,#0x03 
    xor bh,bh
    int 0x10

    ! Show msg1
    ! Cx is the length of msg1
    mov cx,#42
    ! Es:bp is the address of msg1
    mov bp,#msg2
    mov ax,#0x07e0
    mov es,ax
    ! Page 0, attribute7 (normal)
    mov bx,#0x0007
    ! Write string, move cursor
    mov ax,#0x1301
    int 0x10

    mov ax,cs
    mov es,ax
    ! Init ss:sp
    mov ax,#INITSEG
    mov ss,ax
    mov sp,#0xFF00

    ! Get Params 
    ! Get cursor position
    ! Set ds
    mov ax,#INITSEG
    mov ds,ax
    ! Do read position
    mov ah,#0x03
    xor bh,bh
    int 0x10
    ! Move dx to address(ds+0)
    mov [0],dx

    ! Read memory size
    mov ah,#0x88
    int 0x15
    ! Move ax to address(ds+2)
    mov [2],ax

    ! Read table of disk from address which store in 0x41
    ! Set ds and si
    mov ax,#0x0000
    mov ds,ax
    ! Load 4 bits data which is the table address of disk to si and ds from address(di : 0x41)
    lds si, [4*0x41]
    ! Set es and di
    mov ax,#INITSEG
    mov es,ax
    mov di,#0x0004
    ! Rep 16 times
    mov cx,#0x10
    rep
    ! Move address(ds:si) to address(es:di)
    movsb

    ! Be Ready to Print
    mov ax,cs
    mov es,ax
    mov ax,#INITSEG
    mov ds,ax

    ! Cursor Position
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#18
    mov bx,#0x0007
    mov bp,#msg_cursor
    mov ax,#0x1301
    int 0x10
    ! Move cursor'position to dx
    call print_0x
    mov dx,[0]
    call    print_hex
    ! Memory Size
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#14
    mov bx,#0x0007
    mov bp,#msg_memory
    mov ax,#0x1301
    int 0x10
    call print_0x
    mov dx,[2]
    call    print_hex
    ! Add KB
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#4
    mov bx,#0x0007
    mov bp,#msg_kb
    mov ax,#0x1301
    int 0x10
    ! Cyles
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#7
    mov bx,#0x0007
    mov bp,#msg_cyles
    mov ax,#0x1301
    int 0x10
    call print_0x
    mov dx,[4]
    call    print_hex
    ! Heads
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#8
    mov bx,#0x0007
    mov bp,#msg_heads
    mov ax,#0x1301
    int 0x10
    call print_0x
    mov dx,[6]
    call    print_hex
    ! Secotrs
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#10
    mov bx,#0x0007
    mov bp,#msg_sectors
    mov ax,#0x1301
    int 0x10
    call print_0x
    mov dx,[0x12]
    call    print_hex

inf_loop:
    jmp inf_loop

print_0x:
    ! Print 0x
    mov ah,#0x03
    xor bh,bh
    int 0x10
    mov cx,#2
    mov bx,#0x0007
    mov bp,#msg_0x
    mov ax,#0x1301
    int 0x10
    ret

print_hex:
    ! Set cx to control loop 4 times
    mov    cx,#4
print_digit:
    ! Roll the high 4 bits to low 4 bits
    rol    dx,#4
    ! ah = func, al = 00001111(mask code)
    mov    ax,#0x0e0f
    ! Move the low 4 bits to al
    and    al,dl
    ! Add 0x30 to al,it,s the code of ascii
    add    al,#0x30
    ! If num < 10, then print directly,else add 0x07 to make it be equals the ascii of a,b,c,d... 
    cmp    al,#0x3a
    jl     outp
    add    al,#0x07
outp:
    int    0x10
    ! Loop cx(4) times
    loop   print_digit
    ret
print_nl:
    mov    ax,#0xe0d     ! CR
    int    0x10
    mov    al,#0xa     ! LF
    int    0x10
    ret

msg2:
    .byte   13,10
    .ascii  "Now withoutOS is in setup!   ^_^ !  "
    .byte   13,10,13,10
msg_0x:
    .ascii "0x"
msg_cursor:
    .byte 13,10
    .ascii "Cursor position:"
msg_memory:
    .byte 13,10
    .ascii "Memory Size:"
msg_cyles:
    .byte 13,10
    .ascii "Cyls:"
msg_heads:
    .byte 13,10
    .ascii "Heads:"
msg_sectors:
    .byte 13,10
    .ascii "Sectors:"
msg_kb:
    .ascii "  KB"

.org 510
boot_flag:
    .word 0xAA55


