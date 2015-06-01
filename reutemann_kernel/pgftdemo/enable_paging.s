
        .section        .text
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
        mov     %eax, %cr0              # enable page-mappings
        jmp     .+2                     # flush prefetch queue

        pop     %eax
        leave
        ret


