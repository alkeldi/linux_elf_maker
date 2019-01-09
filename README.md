# linux_elf_maker
A library for creating linux ELF files. This is suitable for compilers and assemblers. Note that the ELF `program header table` is not supported (it is the linker's job to do so)
Currently supporting only 32bit elf files. 

 Run Example:

   `gcc example.c; ./a.out; readelf -a output `