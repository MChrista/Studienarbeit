DHBW Kernel
==============
Studienarbeit von Michael Christa und Jonas Polkehn; Kurs TIT12

Install
-------
1. Erstellen des Diskettenimages  
  > mkdosfs -C os.flp 1440
2. Erstellen der Executable  
  > make
3. Excecutable dem Image hinzufügen  
  > dd status=noxfer conv=notrunc if=PATH_TO_EXE of=os.flp
4. Qemu starten  
  > qemu-system-i386 -fda os.flp
