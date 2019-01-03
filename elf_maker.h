#ifndef elf_maker_h
#define elf_maker_h

#include <elf.h>
#include <stdlib.h>
#include <stdio.h>
#include "SLL.h"
#include <string.h>

/* elf and section headers */
typedef Elf32_Ehdr elf_header_t;
typedef Elf32_Shdr elf_section_header_t;

#define SECTION_NAME_MAX_LEN 15 /* maximum string length for a section name */
#define NULL_SECTION 0b001      /* special section */
#define STRINGS_SECTION 0b010   /* special section */
#define SYMBOLS_SECTION 0b100   /* special section */

typedef struct elf_section_t
{
  char section_name[SECTION_NAME_MAX_LEN + 1];
  elf_section_header_t section_header;
  size_t section_data_size;
  void *section_data;
} elf_section_t;

typedef struct special_sections_t
{
  elf_section_t null_section;
  elf_section_t strings_section;
  elf_section_t symbols_section;
} special_sections_t;

typedef struct elf_file_t
{
  elf_header_t file_header;
  special_sections_t special_sections;
  SLL *sections_list;
  unsigned char options;
} elf_file_t;

static elf_section_t *_elf_maker_section_init(char *section_name, void *section_data, size_t data_size)
{
  /* make section */
  elf_section_t *section = (elf_section_t *)malloc(sizeof(elf_section_t));
  memset(section, 0, sizeof(elf_section_t));
  if (section_name)
  {
    strncpy(section->section_name, section_name, SECTION_NAME_MAX_LEN);
  }
  section->section_data = section_data;
  section->section_data_size = data_size;
  section->section_header.sh_size = data_size;
  return section;
}

static void _elf_maker_section_free(elf_section_t *section)
{
  if (!section)
    return;
  free(section);
}

static elf_file_t *elf_maker_init()
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

  /*special section NULL */
  elf_section_t *null_section = &file->special_sections.null_section;
  null_section->section_header.sh_type = SHT_NULL;
  null_section->section_header.sh_link = SHN_UNDEF;
  strncpy(null_section->section_name, ".NULL", SECTION_NAME_MAX_LEN);

  /*special section STRINGS */
  elf_section_t *strings_section = &file->special_sections.strings_section;
  strings_section->section_header.sh_type = SHT_STRTAB;
  strncpy(strings_section->section_name, ".shstrtab", SECTION_NAME_MAX_LEN);

  /* return new file*/
  return file;
}

static void elf_maker_free(elf_file_t *elf_file)
{
  if (!elf_file)
    return;

  /* special sections */
  elf_section_t *strings_section = &elf_file->special_sections.strings_section;
  if (strings_section->section_data)
    free(strings_section->section_data);

  /* other sections */
  SLLNode *iter = elf_file->sections_list->next;
  while (iter)
  {
    elf_section_t *section = iter->data;
    _elf_maker_section_free(section);
    iter = iter->next;
  }
  SLL_free(elf_file->sections_list);
  free(elf_file);
}

static void _elf_maker_add_section(elf_file_t *elf_file, elf_section_t *section)
{
  if (!elf_file || !section)
    return;
  SLL_insert(elf_file->sections_list, section);
  elf_file->file_header.e_shnum++;
}

static elf_section_t *elf_maker_add_section(elf_file_t *elf_file, char *section_name, void *section_data, size_t data_size)
{
  elf_section_t *section = _elf_maker_section_init(section_name, section_data, data_size);
  _elf_maker_add_section(elf_file, section);
  return section;
}

size_t _get_string_section_data_size(elf_file_t *elf_file, unsigned char options)
{
  size_t size = 1; /* because the section is null started */
  SLLNode *iter = elf_file->sections_list->next;
  while (iter)
  {
    elf_section_t *section = iter->data;
    if (!section->section_name)
      continue;
    /* add the length of the string memory */
    size += strlen(section->section_name) + 1;
    /* prepare for the next iteration */
    iter = iter->next;
  }

  /* special sections */
  if ((options & NULL_SECTION) == NULL_SECTION)
  {
    elf_section_t *section = &elf_file->special_sections.null_section;
    if (section->section_name)
      size += strlen(section->section_name) + 1;
  }
  if ((options & STRINGS_SECTION) == STRINGS_SECTION)
  {
    elf_section_t *section = &elf_file->special_sections.strings_section;
    if (section->section_name)
      size += strlen(section->section_name) + 1;
  }
  if ((options & SYMBOLS_SECTION) == SYMBOLS_SECTION)
  {
    elf_section_t *section = &elf_file->special_sections.symbols_section;
    if (section->section_name)
      size += strlen(section->section_name) + 1;
  }
  return size;
}

static void _update_section_important_info(elf_file_t *elf_file, elf_section_t *section, unsigned char options, size_t *section_offset, size_t *name_offset)
{
  elf_section_header_t *section_header = &section->section_header;
  /* section offset in the file image */
  section_header->sh_offset = (section_header->sh_type != SHT_NOBITS && section_header->sh_size > 0) ? *section_offset : 0;
  *section_offset += (section_header->sh_type != SHT_NOBITS) ? section_header->sh_size : 0;

  /* section name offset in the strings section */
  if (section->section_name && options & STRINGS_SECTION == STRINGS_SECTION)
  {
    elf_section_t *strings_section = &elf_file->special_sections.strings_section;
    size_t len = strlen(section->section_name);
    memcpy(strings_section->section_data + *name_offset, section->section_name, len + 1);

    // strings_section->section_data + strings_section
    section_header->sh_name = (len > 0) ? *name_offset : 0;
    *name_offset += (len > 0) ? len + 1 : 0;
  }
}

static void _elf_maker_prepare_for_writing(elf_file_t *elf_file, unsigned char options)
{
  if (!elf_file)
    return;

  SLLInfo *info = elf_file->sections_list->data;

  /* keep same order of options everywhere in code */
  size_t index = 0;
  if ((options & NULL_SECTION) == NULL_SECTION)
    index++;
  if ((options & STRINGS_SECTION) == STRINGS_SECTION)
  {
    elf_file->file_header.e_shstrndx = index++; /* tell the main elf header about the location of the strings section */
    elf_section_t *strings_section = &elf_file->special_sections.strings_section;
    strings_section->section_data_size = _get_string_section_data_size(elf_file, options);
    strings_section->section_header.sh_size = strings_section->section_data_size;
    strings_section->section_data = malloc(strings_section->section_data_size);
    memset(strings_section->section_data, 0, strings_section->section_data_size);
  }
  if ((options & SYMBOLS_SECTION) == SYMBOLS_SECTION)
    index++;

  /* update number of sections */
  elf_file->file_header.e_shnum += index;

  size_t name_offset = 1; /* because the name is null started */
  size_t section_offset = elf_file->file_header.e_shoff + elf_file->file_header.e_shnum * sizeof(elf_section_header_t);

  /* handle special sections first */
  if ((options & NULL_SECTION) == NULL_SECTION)
    _update_section_important_info(elf_file, &elf_file->special_sections.null_section, options, &section_offset, &name_offset);
  if ((options & STRINGS_SECTION) == STRINGS_SECTION)
    _update_section_important_info(elf_file, &elf_file->special_sections.strings_section, options, &section_offset, &name_offset);
  if ((options & SYMBOLS_SECTION) == SYMBOLS_SECTION)
    _update_section_important_info(elf_file, &elf_file->special_sections.symbols_section, options, &section_offset, &name_offset);

  /* handle other sections */
  SLLNode *iter = elf_file->sections_list->next;
  while (iter)
  {
    elf_section_t *section = iter->data;
    _update_section_important_info(elf_file, section, options, &section_offset, &name_offset);
    iter = iter->next;
  }
}

static void elf_maker_write(elf_file_t *elf_file, FILE *output, unsigned char options)
{
  if (!elf_file || !output)
    return;

  /*prepare*/
  _elf_maker_prepare_for_writing(elf_file, options);

  /* write main header */
  fwrite(&elf_file->file_header, 1, sizeof(elf_header_t), output);

  elf_section_t *null_section = &elf_file->special_sections.null_section;
  elf_section_t *strings_section = &elf_file->special_sections.strings_section;
  elf_section_t *symbols_section = &elf_file->special_sections.symbols_section;

  /* write section headers */
  /* keep same order of options everywhere in code */
  if ((options & NULL_SECTION) == NULL_SECTION)
    fwrite(&null_section->section_header, 1, sizeof(elf_section_header_t), output);
  if ((options & STRINGS_SECTION) == STRINGS_SECTION)
    fwrite(&strings_section->section_header, 1, sizeof(elf_section_header_t), output);
  if ((options & SYMBOLS_SECTION) == SYMBOLS_SECTION)
    fwrite(&symbols_section->section_header, 1, sizeof(elf_section_header_t), output);

  SLLNode *iter = elf_file->sections_list->next;
  while (iter)
  {
    elf_section_t *section = iter->data;
    fwrite(&section->section_header, 1, sizeof(elf_section_header_t), output);
    iter = iter->next;
  }

  /* write section data */
  /* keep same order of options everywhere in code */
  if ((options & NULL_SECTION) == NULL_SECTION && null_section->section_data && null_section->section_data_size)
    fwrite(null_section->section_data, null_section->section_data_size, 1, output);
  // if ((options & STRINGS_SECTION) == STRINGS_SECTION && strings_section->section_data && strings_section->section_data_size)
  if ((options & STRINGS_SECTION) == STRINGS_SECTION)
    fwrite(strings_section->section_data, strings_section->section_data_size, 1, output);
  if ((options & SYMBOLS_SECTION) == SYMBOLS_SECTION && symbols_section->section_data && symbols_section->section_data_size)
    fwrite(symbols_section->section_data, symbols_section->section_data_size, 1, output);

  iter = elf_file->sections_list->next;
  while (iter)
  {
    elf_section_t *section = iter->data;
    if (!section->section_data || !section->section_data_size)
      continue;
    fwrite(section->section_data, section->section_data_size, 1, output);
    iter = iter->next;
  }
}
#endif