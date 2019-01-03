#include "elf_maker.h"
int main()
{
  /* elf maker init */
  elf_file_t *elf_file = elf_maker_init();

  /* adding sections */
  char data[] = "this is my data";
  elf_section_t *section = elf_maker_add_section(elf_file, ".section_name", data, strlen(data));
  section->section_header.sh_type = SHT_NOBITS;

  /* prepare output file */
  FILE *ofile = fopen("output", "w");
  if (!ofile)
  {
    fprintf(stderr, "ERORR: couldn't open output file\n");
    exit(1);
  }

  /* write the elf file*/
  elf_maker_write(elf_file, ofile, NULL_SECTION | STRINGS_SECTION);

  /*cleanup */
  fclose(ofile);
  elf_maker_free(elf_file);
}
