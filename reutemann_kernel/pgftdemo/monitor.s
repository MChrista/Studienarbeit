
#==================================================================
# S E C T I O N   D A T A
#==================================================================
        .section        .data

mon_addr:
        .long 0

errmsg: .ascii  "Syntax Error\n"
        .equ    errmsg_len, (.-errmsg)

hlpmsg: .ascii  "Monitor Commands:\n"
        .ascii  "  H           - Help (this text)\n"
        .ascii  "  Q           - Quit monitor\n"
        .ascii  "  R ADDR      - Read from address ADDR\n"
        .ascii  "  W ADDR WORD - Write 32-bit word WORD into ADDR\n\n"
        .ascii  "All addresses/words are in hexadecimal, e.g. 00123ABC\n"
        .ascii  "Leading zeros can be omitted\n"
        .ascii  "\n"
        .equ    hlpmsg_len, (.-hlpmsg)

addrmsg:.ascii  "________: ________\n"
        .equ    addrmsg_len, (.-addrmsg)

#==================================================================
# S E C T I O N   T E X T
#==================================================================
        .section        .text

        .type   run_monitor, @function
        .global run_monitor
        .extern kgets
run_monitor:
        enter   $268, $0
        pusha
        push    %gs

        lea     -256(%ebp), %esi
        mov     %esi, mon_addr

        #----------------------------------------------------------
        # setup GS segment register for linear addressing
        #----------------------------------------------------------
        mov     $linDS, %ax
        mov     %ax, %gs

        xor     %ecx, %ecx
.Lloop:
        lea     -256(%ebp), %esi
        call    kgets
        test    %eax, %eax
        mov     %eax, %ecx          # buffer index
        jz      .Lmonitor_exit
        movb    (%esi), %al
        # commands without parameters
        cmpb    $'Q', %al
        je      .Lmonitor_exit
        cmpb    $'H', %al
        je      .Lhelp
        cmpb    $'#', %al
        je      .Lloop
        # commands that require parameters
        cmp     $3, %cl
        jb      .Lerror
        cmpb    $'W', %al
        je      .Lwrite_addr
        cmpb    $'R', %al
        je      .Lread_addr
        cmpb    $'D', %al
        je      .Ldump_addr
.Lerror:
        mov     %ecx, -260(%ebp)
        lea     errmsg, %esi
        mov     $errmsg_len, %ecx
        call    screen_write
        mov     -260(%ebp), %ecx
        jmp     .Lloop
.Lhelp:
        mov     %ecx, -260(%ebp)
        lea     hlpmsg, %esi
        mov     $hlpmsg_len, %ecx
        call    screen_write
        mov     -260(%ebp), %ecx
        jmp     .Lloop
.Lwrite_addr:
        inc     %esi
        call    hex2int
        mov     %eax, %edi

        call    hex2int
        movl    %eax, %gs:(%edi)
        jmp     .Lloop
.Lread_addr:
        mov     %ecx, -260(%ebp)
        inc     %esi
        call    hex2int
        mov     %eax, -264(%ebp)        # store address on stack

        lea     addrmsg, %edi           # pointer to output string
        mov     $8, %ecx                # number of output digits
        call    int_to_hex

        mov     -264(%ebp), %edi
        movl    %gs:(%edi), %eax

        lea     addrmsg+10, %edi        # pointer to output string
        mov     $8, %ecx                # number of output digits
        call    int_to_hex

        lea     addrmsg, %esi           # message-offset
        mov     $addrmsg_len, %ecx      # message-length
        call    screen_write
        mov     -260(%ebp), %ecx
        jmp     .Lloop
.Ldump_addr:
        jmp     .Lloop
.Lmonitor_exit:

        pop     %gs
        popa
        leave
        ret


#-------------------------------------------------------------------
# FUNCTION:   hex2int
#
# PURPOSE:    Convert a hexadecimal ASCII string into an integer
#
# PARAMETERS: (via register)
#             ESI - pointer to input string
#
# RETURN:     EAX - converted integer
#             ESI points to the next character of the hex string
#
#-------------------------------------------------------------------
        .type   hex2int, @function
hex2int:
        enter   $0, $0
        push    %edx

        xor     %eax, %eax
.Lspcloop:
        mov     (%esi), %dl
        test    %dl, %dl
        jz      .Lexit
        cmp     $'\n', %dl
        jz      .Lexit
        cmp     $'\r', %dl
        jz      .Lexit
        inc     %esi
        cmp     $' ', %dl
        je      .Lspcloop
        dec     %esi
.Lhexloop:
        mov     (%esi), %dl
        test    %dl, %dl
        jz      .Lexit
        cmp     $'\n', %dl
        jz      .Lexit
        cmp     $'\r', %dl
        jz      .Lexit
        cmp     $' ', %dl
        jz      .Lexit
        cmp     $0, %dl
        jb      .Lexit
        # dl >= '0'
        cmp     $'f', %dl
        ja      .Lexit
        # dl >= '0' && dl <= 'f'
        cmp     $'9', %dl
        mov     $'0', %dh
        jbe     .Lconv_digit     # dl >= '0' && dl <= '9'
        # dl > '9' && dl <= 'f'
        cmp     $'A', %dl
        jb      .Lexit
        # dl >= 'A' && dl <= 'f'
        cmp     $'F', %dl
        mov     $'A'-10, %dh
        jbe     .Lconv_digit     # dl => 'A' && dl <= 'F'
        # dl > 'F' && dl <= 'f'
        cmp     $'a', %dl
        jb      .Lexit
        # dl >= 'a' && dl <= 'f'
        mov     $'a'-10, %dh
.Lconv_digit:
        sub     %dh, %dl        # convert hex digit to int 0..15
        shl     $4, %eax        # multiply result by 16
        movzxb  %dl, %edx
        add     %edx, %eax      # add digit value to result
        inc     %esi
        jmp     .Lhexloop
.Lexit:
        inc     %esi
        pop     %edx
        leave
        ret

