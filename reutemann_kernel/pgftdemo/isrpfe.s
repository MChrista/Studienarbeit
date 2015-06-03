
#==================================================================
#=========    TRAP-HANDLER FOR PAGE-FAULT EXCEPTIONS    ===========
#==================================================================
#
#-----------------------------------------------------------------
# Stack Frame Layout
#-----------------------------------------------------------------
#
#                 Byte 0
#                      V
#    +-----------------+
#    |     EFLAGS      |  +64
#    +-----------------+
#    |       CS        |  +60
#    +-----------------+
#    |       EIP       |  +56
#    +-----------------+
#    |    Error Code   |  +52
#    +-----------------+
#    |      INT ID     |  +48
#    +-----------------+
#    |   General Regs  |
#    | EAX ECX EDX EBX |  +32
#    | ESP EBP ESI EDI |  +16
#    +-----------------+
#    |  Segment  Regs  |
#    |   DS ES FS GS   |  <-- ebp
#    +=================+
#
#-----------------------------------------------------------------

#-----------------------------------------------------------------
# S E C T I O N   D A T A
#-----------------------------------------------------------------
        .section    .data
        #----------------------------------------------------------
        # output string for faulting address and physical address
        #----------------------------------------------------------
pgftmsg:
        .ascii  "Page fault @ 0x"
pgftaddr:
        .ascii  "________ -> "
pgphaddr:
        .ascii  "________\n"
        .equ    pgftmsg_len, (.-pgftmsg)

#-----------------------------------------------------------------
# S E C T I O N   T E X T
#-----------------------------------------------------------------
        .section    .text
        .type       isrPFE, @function
        .globl      isrPFE
        .extern     int_to_hex
        .extern     screen_write
        .extern     pfhandler
        .code32
        .align   16
#------------------------------------------------------------------
isrPFE:
        #----------------------------------------------------------
        # push interrupt id onto stack for register/stack dump
        # in case page fault cannot be resolved
        # 14: Page Fault Exception (With Error Code!)
        #----------------------------------------------------------
        pushl   $14

        #-----------------------------------------------------------
        # push general-purpose and all data segment registers onto
        # stack in order to preserve their value and also for display
        #-----------------------------------------------------------
        pushal
        pushl   %ds
        pushl   %es
        pushl   %fs
        pushl   %gs
        mov     %esp, %ebp              # store current stack pointer

        #----------------------------------------------------------
        # setup segment registers
        #----------------------------------------------------------
        mov     $privDS, %ax
        mov     %ax, %ds

        mov     %cr2, %eax              # faulting address
        lea     pgftaddr, %edi
        mov     $8, %ecx
        call    int_to_hex

        mov     %cr2, %eax
        pushl   %eax
        call    pfhandler
        add     $4, %esp
        mov     %eax, %ebx
        mov     16(%ebx), %eax          # get physical address
        lea     pgphaddr, %edi
        mov     $8, %ecx
        call    int_to_hex

        lea     pgftmsg, %esi           # message-offset into ESI
        mov     $pgftmsg_len, %ecx      # message-length into ECX
        call    screen_write

        #----------------------------------------------------------
        # TODO:
        # flags not yet supported
        # just make a simple check of the physical address
        # 0xffffffff indicates that the page fault could not be
        # resolved
        #----------------------------------------------------------
        cmpl    $0xffffffff, 16(%ebx)
        jne     pfe_exit

        #----------------------------------------------------------
        # write the faulting address into the EAX value on the stack
        #----------------------------------------------------------
        mov     %cr2, %eax
        mov     %eax, 44(%ebp)
        #----------------------------------------------------------
        # print register values
        #----------------------------------------------------------
        pushl   $1<<11+1<<13+1<<14       # highlight some registers
        pushl   $50
        pushl   $0x9e7070
        pushl   $INT_NUM-2
        pushl   $0
        pushl   $intname
        pushl   %ebp
        call    print_stacktrace
        add     $7*4, %esp

        #----------------------------------------------------------
        # modify the interrupt return address on the stack in order
        # to abort the instruction accessing the faulting address
        # and jump to a different code location instead
        #----------------------------------------------------------
        lea     pfcontinue, %eax
        mov     %eax, 56(%ebp)

pfe_exit:
        #----------------------------------------------------------
        # restore the values to the registers we've modified here
        #----------------------------------------------------------
        popl    %gs
        popl    %fs
        popl    %es
        popl    %ds
        popal

        #----------------------------------------------------------
        # remove interrupt id and error code from stack
        #----------------------------------------------------------
        add     $8, %esp

        iret

