Studienarbeit: DHBW Kernel
==============

|Gruppenmitglieder| Betreuer |
|-----------------|----------|
|Michael Christa  |Herr Dr. Reutemann |
|Jonas Polkehn    |


Install
-------
1. Erstellen des Diskettenimages  
`$ mkdosfs -C os.flp 1440`
2. Erstellen der Executable  
`$ make`
3. Excecutable dem Image hinzufügen  
`$ dd status=noxfer conv=notrunc if=PATH_TO_EXE of=os.flp`
4. Qemu starten  
`$ qemu-system-i386 -fda os.flp`

Der Ablauf ist [hier](http://bumble.sourceforge.net/books/osdev/osdev-book.txt.x.html "Befehle")
näher beschrieben

Debugging
---------
1. Starten von QEMU  
`$ qemu-system-i386 -s -S -fda os.flp`
2. Starten des gdb  
`$ gdb`  
`(gdb) target remote localhost:1234`  
`(gdb) continue`
