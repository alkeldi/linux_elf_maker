#include "elf_maker.h"
#include <string.h>
int main()
{
  /* elf maker init */
  elf_file_t *elf_file = elf_maker_init();

  /* add null section */
  elf_section_t *null_section = elf_maker_add_section(elf_file, ".null");

  /* add strings section (can't be the first section)*/
  elf_section_t *strings_section = elf_maker_add_section(elf_file, ".shstrtab");
  strings_section->header.sh_type = SHT_STRTAB;

  /* tell elf file about the index of the strings section */
  elf_file->header.e_shstrndx = 1;

  /* finalize the string section */
  elf_maker_add_section_entry(strings_section, "\0", 1);
  SLLNode *sections_iter = elf_file->sections->next;
  while (sections_iter)
  {
    elf_section_t *section = sections_iter->data;
    if (section->name)
    {
      section->header.sh_name = strings_section->header.sh_size;
      elf_maker_add_section_entry(strings_section, section->name, strlen(section->name) + 1);
    }
    sections_iter = sections_iter->next;
  }

  /* prepare output file */
  FILE *ofile = fopen("output", "w");
  if (!ofile)
  {
    fprintf(stderr, "ERORR: couldn't open output file\n");
    exit(1);
  }

  /* write the elf file*/
  elf_maker_write(elf_file, ofile);

  /*cleanup */
  fclose(ofile);
  elf_maker_free(elf_file);
}
