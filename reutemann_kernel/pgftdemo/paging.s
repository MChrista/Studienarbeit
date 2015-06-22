
#==================================================================
# S E C T I O N   T E X T
#==================================================================
        .section        .text
        .code32


#-------------------------------------------------------------------
# FUNCTION:   enable_paging
#
# PURPOSE:    Enable paging support by the microprocessor
#
# C Call:     void enable_paging(void)
#
# PARAMETERS: none
#
# RETURN:     none
#
#-------------------------------------------------------------------
        .type           enable_paging, @function
        .globl          enable_paging
        .extern         LD_DATA_START   # constant defined in ldscript
enable_paging:
        enter   $0, $0
        push    %eax

        #----------------------------------------------------------
        # setup page-directory address in control register CR3
        #----------------------------------------------------------
        add     $LD_DATA_START, %eax    # add .data start address
        mov     %eax, %cr3              # goes into CR3 register

        #----------------------------------------------------------
        # turn on paging (by setting bit #31 in register CR0)
        #----------------------------------------------------------
        mov     %cr0, %eax              # current machine status
        bts     $31, %eax               # turn on PG-bit's image
        bts     $30, %eax               # disable caching
        bts     $16, %eax               # enable write protection
        mov     %eax, %cr0              # enable page-mappings
        jmp     .+2                     # flush prefetch queue

        pop     %eax
        leave
        ret


#-------------------------------------------------------------------
# FUNCTION:   disable_paging
#
# PURPOSE:    Disable paging support by the microprocessor
#
# C Call:     void disable_paging(void)
#
# PARAMETERS: none
#
# RETURN:     none
#
#-------------------------------------------------------------------
        .type           disable_paging, @function
        .globl          disable_paging
disable_paging:
        enter   $0, $0
        push    %eax

        #----------------------------------------------------------
        # turn off paging (by clearing bit #31 in register CR0)
        #----------------------------------------------------------
        mov     %cr0, %eax              # current machine status
        btc     $31, %eax               # turn on PG-bit's image
        mov     %eax, %cr0              # enable page-mappings
        jmp     .+2                     # flush prefetch queue

        #----------------------------------------------------------
        # invalidate the CPU's Translation Lookaside Buffer
        #----------------------------------------------------------
        xor     %eax, %eax              # setup "dummy" value
        mov     %eax, %cr3              # and write it to CR3

        pop     %eax
        leave
        ret


#-------------------------------------------------------------------
# FUNCTION:   invalidate_addr
#
# PURPOSE:    Invalidates the TLB entry associated with the virtual
#             address provided as argument
#
# C Call:     void invalidate_addr(unsigned long addr)
#
# PARAMETERS: (via stack - C style)
#             addr - virtual address to invalidate
#
# RETURN:     none
#
#-------------------------------------------------------------------
        .type           invalidate_addr, @function
        .globl          invalidate_addr
invalidate_addr:
        enter   $0, $0
        push    %eax
        push    %gs

        #----------------------------------------------------------
        # setup GS segment register for linear addressing
        #----------------------------------------------------------
        mov     $linDS, %ax
        mov     %ax, %gs

        #----------------------------------------------------------
        # invalidate the TLB entry for the given linear address
        #----------------------------------------------------------
        mov     8(%ebp), %eax
        invlpg  %gs:(%eax)

        pop     %gs
        pop     %eax
        leave
        ret

