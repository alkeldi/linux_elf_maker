# linux_elf_maker
A library for creating linux ELF files. This is suitable for compilers and assemblers. Note that the ELF `program header table` is not supported (it is the linker's job to do so)
Currently supporting only 32bit elf files. 

 Run Example:

  `rm -f a.out output; gcc *.c;./a.out; readelf -a output`

Link &run Output:

  `ld output`
  `./a.out; echo $?`