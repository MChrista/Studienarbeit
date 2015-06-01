

=== Memory Layout ===

 0x300000  +---------------------------+
 0x2FFFFF  |                           |
           |   Memory area used for    |
           |   allocation of  pages    |
           |                           |
 0x200000  +---------------------------+
 0x1FFFFF  |                           |
           |         RAM Disk          |
           |                           |
           |     Example Programs      |
           |        ELF Images         |
 0x100000  +---------------------------+
  0xFFFFF  |                           |
           |      Reserved (BIOS)      |
           |                           |
  0xC0000  +---------------------------+
           |                           |
           |   CGA Text Video Buffer   |
           |      4 Pages (25x80)      | sel_es
           |                           |
  0xB8000  +---------------------------+
           |                           |
           |     Reserved (Video)      |
           |                           |
  0x9FC00  +---------------------------+
           |                           |
           .                           .
           .                           .
           |    Kernel  .data .bss     | privDS
           |                           |
  0x20000  +---------------------------+
           |                           |
           |       Kernel .text        | privCS
           |                           |
  0x10000  +---------------------------+
           |  |    Kernel Stack     |  |
           |  V  (Protected Mode)   V  | privSS
           |                           |
  0x0C000  +---------------------------+
           |       Unused Memory       |
           |                           |
  0x07e00  +---------------------------+
           |        Boot Sector        |
           |         512 Bytes         |
  0x07C00  +---------------------------+
           |  |   Real Mode Stack   |  |
           |  V                     V  |
           |                           |
  0x00500  +---------------------------+
           |       BIOS Data Area      |
           |           (BDA)           | sel_bs
  0x00400  +---------------------------+
           |         Real Mode         |
           |   Interrupt Vector Table  |
  0x00000  +---------------------------+

