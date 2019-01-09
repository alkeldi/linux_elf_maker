/*************************************************** Includes ********************************************************/
#include "../include/elf_maker.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/*********************************************** Public Functions ****************************************************/
/* create a new elf file struct */
elf_file_t *elf_maker_init()
{
  /* init the header */
  elf_header_t header;
  memset(&header, 0, sizeof(elf_header_t));
  header.e_ident[EI_MAG0] = ELFMAG0;       /* magic start of elf file '0x7F' */
  header.e_ident[EI_MAG1] = ELFMAG1;       /* magic start of elf file E */
  header.e_ident[EI_MAG2] = ELFMAG2;       /* magic start of elf file L */
  header.e_ident[EI_MAG3] = ELFMAG3;       /* magic start of elf file F */
  header.e_ident[EI_CLASS] = ELFCLASS32;   /* architecture version (32bit) */
  header.e_ident[EI_DATA] = ELFDATA2LSB;   /* endianness (little endian => LSB) */
  header.e_ident[EI_VERSION] = EV_CURRENT; /* elf header version (current = 1) */
  header.e_ident[EI_OSABI] = 0;            /* target os => linux ignores this, so use 0 */
  header.e_ident[EI_ABIVERSION] = 0;       /* target os abreviation => linux ignores this, so use 0 */
  header.e_type = ET_REL;                  /* Object file type (relocateable) */
  header.e_machine = EM_386;               /* instruction set (x86 for now) */
  header.e_version = EV_CURRENT;           /* elf header version (current = 1) */
  header.e_ehsize = sizeof(elf_header_t);  /* size of the elf header */
  header.e_shoff = header.e_ehsize;        /* section headers offset */
  //TODO: FIX OR REMOVE;  header.e_phentsize = sizeof(Elf32_Phdr); /* size of entry in the program header table */
  header.e_shentsize = sizeof(Elf32_Shdr); /* size of entry in the section header table */

  /* add the elf header to the elf file */
  elf_file_t *file = (elf_file_t *)malloc(sizeof(elf_file_t));
  memset(file, 0, sizeof(elf_file_t));
  file->file_header = header;
  file->sections_list = SLL_init();

  /* return new file*/
  return file;
}

/* cleanup the given elf_file */
void elf_maker_free(elf_file_t *elf_file)
{
  if (!elf_file)
    return;

  SLLNode *iter = elf_file->sections_list->next;
  while (iter)
  {
    /* free sections */
    elf_section_t *section = iter->data;
    _elf_maker_section_free(section);
    iter = iter->next;
  }
  SLL_free(elf_file->sections_list);
  free(elf_file);
  elf_file = NULL;
}

/* add a section into an existing elf_file */
elf_section_t *elf_maker_add_section(elf_file_t *elf_file, char *section_name, void *section_data, size_t data_size)
{
  if (!elf_file)
    return NULL;

  /* make section */
  elf_section_t *section = (elf_section_t *)malloc(sizeof(elf_section_t));
  memset(section, 0, sizeof(elf_section_t));
  if (section_name)
    strncpy(section->section_name, section_name, SECTION_NAME_MAX_LEN);

  section->section_data = section_data;
  section->section_header.sh_size = data_size;

  /* add section to the elf file */
  SLL_insert(elf_file->sections_list, section);
  elf_file->file_header.e_shnum++;

  /*return the created section */
  return section;
}

/* write elf to an output file */
void elf_maker_write(elf_file_t *elf_file, FILE *output)
{
  if (!elf_file || !output)
    return;

  /*prepare*/
  _elf_maker_prepare_for_writing(elf_file);

  /* write main header */
  fwrite(&elf_file->file_header, 1, sizeof(elf_header_t), output);

  /* write sections headers */
  SLLNode *iter = elf_file->sections_list->next;
  while (iter)
  {
    elf_section_t *section = iter->data;
    fwrite(&section->section_header, 1, sizeof(elf_section_header_t), output);
    iter = iter->next;
  }

  /* write sections data */
  iter = elf_file->sections_list->next;
  while (iter)
  {
    elf_section_t *section = iter->data;
    if (!section->section_data)
    {
      iter = iter->next;
      continue;
    }
    fwrite(section->section_data, section->section_header.sh_size, 1, output);
    iter = iter->next;
  }
}


/*********************************************** Private Functions ***************************************************/

/* free a given section */
void _elf_maker_section_free(elf_section_t *section)
{
  if (!section)
    return;
  free(section);
  section = NULL;
}

/* prepare for writing the elf file */
void _elf_maker_prepare_for_writing(elf_file_t *elf_file)
{
  if (!elf_file)
    return;

  size_t section_offset = elf_file->file_header.e_shoff + elf_file->file_header.e_shnum * sizeof(elf_section_header_t);
  SLLNode *iter = elf_file->sections_list->next;
  while (iter)
  {
    elf_section_t *section = iter->data;
    if (section->section_header.sh_type != SHT_NOBITS)
    {
      section->section_header.sh_offset = section_offset;
      section_offset += section->section_header.sh_size;
    }
    /* next iteration */
    iter = iter->next;
  }
}