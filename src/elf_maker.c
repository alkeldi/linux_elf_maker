/*************************************************** Includes ********************************************************/
#include "elf_maker.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*********************************************** Public Functions ****************************************************/
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
  header.e_shentsize = sizeof(Elf32_Shdr); /* size of entry in the section header table */

  /* add the elf header to the elf file */
  elf_file_t *file = (elf_file_t *)malloc(sizeof(elf_file_t));
  memset(file, 0, sizeof(elf_file_t));
  file->header = header;
  file->sections = SLL_init();

  /* return new file*/
  return file;
}

elf_section_t *elf_maker_add_section(elf_file_t *elf_file, char *name)
{
  /* error checking */
  if (!elf_file)
    return NULL;

  /* make section */
  elf_section_t *section = (elf_section_t *)malloc(sizeof(elf_section_t));
  memset(section, 0, sizeof(elf_section_t));

  /* section name */
  if (name)
  {
    size_t len = strlen(name);
    section->name = malloc(len + 1);
    strcpy(section->name, name);
    section->name[len] = 0;
  }

  /* section entries */
  section->entries = SLL_init();

  /* add section to the elf file */
  SLL_insert(elf_file->sections, section);

  /*return the created section */
  return section;
}

elf_section_entry_t *elf_maker_add_section_entry(elf_section_t *section, void *data, size_t size)
{
  /* error checking */
  if (!section || !data || !size)
    return NULL;

  /* make entry */
  elf_section_entry_t *entry = (elf_section_entry_t *)malloc(sizeof(elf_section_entry_t));
  memset(entry, 0, sizeof(elf_section_entry_t));

  /* entry size */
  entry->size = size;

  /* entry data */
  entry->data = malloc(size);
  memcpy(entry->data, data, size);

  /* add entry to the section */
  section->header.sh_size += size;
  SLL_insert(section->entries, entry);

  /*return the created entry */
  return entry;
}

void elf_maker_write(elf_file_t *elf_file, FILE *output)
{
  if (!elf_file || !output)
    return;

  /* prepare for writing */
  _elf_maker_prepare_for_writing(elf_file);

  /* write main elf header */
  fwrite(&elf_file->header, 1, sizeof(elf_header_t), output);

  /* write sections headers */
  SLLNode *sections_iter = elf_file->sections->next;
  while (sections_iter)
  {
    elf_section_t *section = sections_iter->data;
    fwrite(&section->header, 1, sizeof(elf_section_header_t), output);
    sections_iter = sections_iter->next;
  }

  /* write sections entries */
  sections_iter = elf_file->sections->next;
  while (sections_iter)
  {
    elf_section_t *section = sections_iter->data;
    SLLNode *section_entries_iter = section->entries->next;
    while (section_entries_iter)
    {
      elf_section_entry_t *entry = section_entries_iter->data;
      fwrite(entry->data, 1, entry->size, output);
      section_entries_iter = section_entries_iter->next;
    }
    sections_iter = sections_iter->next;
  }
}

void elf_maker_free(elf_file_t *elf_file)
{
  if (!elf_file)
    return;

  /* free sections */
  SLLNode *sections_iter = elf_file->sections->next;
  while (sections_iter)
  {
    elf_section_t *section = sections_iter->data;
    _elf_maker_section_free(section);
    sections_iter = sections_iter->next;
  }
  SLL_free(elf_file->sections);
  elf_file->sections = NULL;

  /* free the file */
  free(elf_file);
  elf_file = NULL;
}
/*********************************************** Private Functions ***************************************************/
void _elf_maker_prepare_for_writing(elf_file_t *elf_file)
{
  if (!elf_file)
    return;

  /* get sections info */
  SLLInfo *sections_info = elf_file->sections->data;

  /* update number of sections */
  elf_file->header.e_shnum = sections_info->size;

  /* update sections offset */
  size_t section_offset = elf_file->header.e_shoff + elf_file->header.e_shnum * sizeof(elf_section_header_t);
  SLLNode *sections_iter = elf_file->sections->next;
  while (sections_iter)
  {
    elf_section_t *section = sections_iter->data;
    if (section->header.sh_size && section->header.sh_type != SHT_NOBITS)
    {
      section->header.sh_offset = section_offset;
      section_offset += section->header.sh_size;
    }
    /* next iteration */
    sections_iter = sections_iter->next;
  }
}

void _elf_maker_section_free(elf_section_t *section)
{
  if (!section)
    return;

  /* free the name */
  if (section->name)
  {
    free(section->name);
    section->name = NULL;
  }

  /* free the entries */
  SLLNode *section_entries_iter = section->entries->next;
  while (section_entries_iter)
  {
    elf_section_entry_t *entry = section_entries_iter->data;
    _elf_maker_section_entry_free(entry);
    section_entries_iter = section_entries_iter->next;
  }
  SLL_free(section->entries);
  section->entries = NULL;

  /* free the section */
  free(section);
  section = NULL;
}

void _elf_maker_section_entry_free(elf_section_entry_t *entry)
{
  if (!entry)
    return;

  /* free data */
  if (entry->data)
  {
    free(entry->data);
    entry->data = NULL;
  }

  /* free entry */
  free(entry);
  entry = NULL;
}