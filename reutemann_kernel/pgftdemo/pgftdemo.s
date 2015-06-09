#-----------------------------------------------------------------
# pgftdemo.s
#
#-----------------------------------------------------------------


#==================================================================
# S I G N A T U R E
#==================================================================
        .section        .signature, "a", @progbits
        .long   progname_size
progname:
        .ascii  "PGFTDEMO"
        .equ    progname_size, (.-progname)
        .byte   0


#==================================================================
# S E C T I O N   D A T A
#==================================================================

        .section        .data

        .equ    DATA_START, 0x20000

#------------------------------------------------------------------
# G L O B A L   D E S C R I P T O R   T A B L E
#------------------------------------------------------------------
        .align  16
        .global theGDT
theGDT:
        .include "comgdt.inc"
        #----------------------------------------------------------
        # Code/Data, 32 bit, 4kB, Priv 0, Type 0x02, 'Read/Write'
        # Base Address: 0x00000000   Limit: 0x000fffff
        .equ    linDS, (.-theGDT)       # selector for data
        .globl  linDS
        .quad   0x00CF92000000FFFF      # data segment-descriptor
        #----------------------------------------------------------
        .equ    limGDT, (. - theGDT)-1  # our GDT's segment-limit
#------------------------------------------------------------------
        # image for GDTR register
        .align  16
        .global regGDT
regGDT: .word   limGDT
        .long   theGDT+DATA_START       # create linear address

#------------------------------------------------------------------
# I N T E R R U P T   D E S C R I P T O R   T A B L E
#------------------------------------------------------------------
        .align  16
        .global theIDT
        #----------------------------------------------------------
theIDT: # allocate 256 gate-descriptors
        #----------------------------------------------------------
        .quad   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        #----------------------------------------------------------
        # General Protection Exception (0x0D) gate descriptor
        #----------------------------------------------------------
        .word   isrGPF, privCS, 0x8E00, 0x0000
        #----------------------------------------------------------
        # Page Fault Exception (0x0E) gate descriptor
        .word   isrPFE, privCS, 0x8E00, 0x0000
        #----------------------------------------------------------
        # allocate free space for the remaining unused descriptors
        #----------------------------------------------------------
        .zero   256*8 - (.-theIDT)
        .equ    limIDT, (.-theIDT)-1    # this IDT's segment_limit

        #----------------------------------------------------------
        # image for IDTR register
        #----------------------------------------------------------
        .align  16
        .global regIDT
regIDT: .word   limIDT
        .long   theIDT+DATA_START       # create linear address

#------------------------------------------------------------------

        #----------------------------------------------------------
        # output string for page directory address
        #----------------------------------------------------------
pgdirmsg:
        .ascii  "Page Directory is at linear address 0x"
pgdiraddr:
        .ascii  "________\n"
        .equ    pgdirmsg_len, (.-pgdirmsg)

rwchar: .ascii  "RW"

        #----------------------------------------------------------
        # output string for linear address we attempt to access
        #----------------------------------------------------------
addrmsg:.ascii  "________   "
pgflags:.ascii  "____\n"
        .equ    addrmsg_len, (.-addrmsg)

        #----------------------------------------------------------
        # address samples
        #
        # NOTE: bit 31 is used to indicate write access
        #----------------------------------------------------------
samples:#.long   0x00010000, 0x000100ff, 0x00020000, 0x00020abc
        #.long   0x000B8000, 0x000110ff, 0x08048000, 0x08048000
        #.long   0x08051c00, 0x08050abc, 0x60000000, 0x08048fff
        .long   0x08080000, 0x08048123, 0x08049321, 0x08049004
        .long   0x080a0fff, 0x08049321, 0x08049004, 0x08050abc
        .long   0x880a0fff, 0x880a0fff, 0x880a0004, 0x88048FFF
        .long   0x98050abc
        .long   0x00000000

oldesp: .long   0x00000000

#==================================================================
# S E C T I O N   T E X T
#==================================================================
        .section        .text
        .code32
#------------------------------------------------------------------
# M A I N   F U N C T I O N
#------------------------------------------------------------------
        .type   main, @function
        .global main
        .extern init_paging
        .extern enable_paging
        .extern int_to_hex
        .extern screen_write
        .extern screen_sel_page
main:
        enter   $0, $0
        pushal
        push    %gs
        mov     %esp, oldesp    # save stack pointer

        #----------------------------------------------------------
        # Segment register usage (provided by start.o):
        #   CS - Code Segment
        #   DS - Data Segment
        #   SS - Stack Segment
        #   ES - CGA Video Memory
        #----------------------------------------------------------

        #----------------------------------------------------------
        # initialise multi-page console
        #----------------------------------------------------------
        xor     %eax, %eax       # select page #0
        call    screen_sel_page

        #----------------------------------------------------------
        # initialise page directory and page tables
        # page directory address will be returned in EAX
        #----------------------------------------------------------
        call    init_paging

        #----------------------------------------------------------
        # enable paging
        # page directory address expected in EAX
        #----------------------------------------------------------
        call    enable_paging

        #----------------------------------------------------------
        # print the page directory address
        #----------------------------------------------------------
        mov     %cr3, %eax
        lea     pgdiraddr, %edi
        mov     $8, %ecx
        call    int_to_hex
        lea     pgdirmsg, %esi          # message-offset into ESI
        mov     $pgdirmsg_len, %ecx     # message-length into ECX
        call    screen_write

        #----------------------------------------------------------
        # setup GS segment register for linear addressing
        #----------------------------------------------------------
        mov     $linDS, %ax
        mov     %ax, %gs

        #----------------------------------------------------------
        # read address samples from array
        # end of array is indicated by zero address
        #----------------------------------------------------------
        xor     %ecx, %ecx
read_samples:
        mov     samples(,%ecx,4), %eax
        test    %eax, %eax
        jz      read_done
        push    %ecx                    # save array index
        push    %eax                    # save sample address

        #----------------------------------------------------------
        # convert and print the linear address in EAX
        #----------------------------------------------------------
        mov     %eax, %edx
        shr     $31, %edx
        mov     rwchar(%edx), %dl
        mov     %dl, addrmsg+9
        and     $0x7fffffff, %eax       # mask out MSB
        lea     addrmsg, %edi           # pointer to output string
        mov     $8, %ecx                # number of output digits
        call    int_to_hex

        call    get_pg_flags
        mov     %eax, pgflags

        lea     addrmsg, %esi           # message-offset
        mov     $addrmsg_len, %ecx      # message-length
        call    screen_write

        #----------------------------------------------------------
        # now try to access the address
        #----------------------------------------------------------
        pop     %eax                    # restore sample address
        test    $0x80000000, %eax
        jnz     do_write
        and     $0x7fffffff, %eax       # mask out MSB
        mov     %gs:(%eax), %ebx        # read access
        jmp     skip_write
do_write:
        and     $0x7fffffff, %eax       # mask out MSB
        notl    %gs:(%eax)              # write access
skip_write:
        popl    %ecx                    # restore array index
        inc     %ecx
        jmp     read_samples
read_done:

        .type   pfcontinue, @function
        .global pfcontinue
pfcontinue:
        mov     oldesp, %esp      # restore stack pointer

        #----------------------------------------------------------
        # in order to succesfully go back to the boot loader we
        # have to disable paging first
        #----------------------------------------------------------
        call    disable_paging

        pop     %gs
        popal
        leave
        ret

#------------------------------------------------------------------
# read the paging flags for the given linear address
#       %eax (in): linear address
#
# return: flags in %eax
#------------------------------------------------------------------
get_pg_flags:
        enter   $4, $0
        push    %edx
        push    %esi

        mov     %eax, %edx
        mov     %cr3, %esi        # linear page directory address

        # calculate segmented page table address
        shr     $22, %edx            # page directory entry
        sub     $LD_DATA_START, %esi # segmented page directory address
        mov     (%esi,%edx,4), %esi  # linear address of page table
        test    $1, %esi             # page table present?
        jz      table_not_mapped     # yes, there is no table mapped

        sub     $LD_DATA_START, %esi # segmented page table address
        and     $0xfffff000, %esi
        mov     %eax, %edx
        shr     $12, %edx            # page table entry
        and     $0x3ff, %edx
        lea     (%esi,%edx,4), %esi
        mov     (%esi), %edx         # linear address of page
        and     $0xfff, %edx
        mov     $0x2e202020, %eax
        test    $1, %edx             # check 'present' bit
        jz      get_pg_flags_end
        mov     $'P', %al
        shl     $8, %eax
        mov     $'R', %al
        test    $2, %edx             # check 'read/write' bit
        jz      read_only
        mov     $'W', %al
read_only:
        shl     $8, %eax
        mov     $'a', %al
        test    $1<<5, %edx          # check 'accessed' bit
        jz      not_accessed
        mov     $'A', %al
        xorl    $1<<5, (%esi)        # flip 'accessed' bit
not_accessed:
        shl     $8, %eax
        mov     $'d', %al
        test    $1<<6, %edx          # check 'dirty' bit
        jz      get_pg_flags_end
        mov     $'D', %al
        jmp     get_pg_flags_end

table_not_mapped:
        mov     $0x20202020, %eax

get_pg_flags_end:

        pop     %esi
        pop     %edx
        leave
        ret

#------------------------------------------------------------------
# disable interrupts and halt processor
# will be called by the General Protection Fault ISR
#------------------------------------------------------------------
        .type   bail_out, @function
        .global bail_out
bail_out:
        cli
        hlt
#------------------------------------------------------------------
        .end

