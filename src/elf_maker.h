#ifndef elf_maker_h
#define elf_maker_h

/*************************************************** Includes ********************************************************/
#include <elf.h>
#include "../misc/SLL.h"
#include <stdio.h>

/*********************************************** Types Definitions ***************************************************/
/* elf and section headers - wrapers*/
typedef Elf32_Ehdr elf_header_t;
typedef Elf32_Shdr elf_section_header_t;

/* elf file struct */
typedef struct elf_file_t
{
  elf_header_t header;
  SLL *sections;
} elf_file_t;

/* elf section struct */
typedef struct elf_section_t
{
  char *name;
  elf_section_header_t header;
  SLL *entries;
} elf_section_t;

/* section entry struct */
typedef struct elf_section_entry_t
{
  size_t size;
  void *data;
} elf_section_entry_t;

/*********************************************** Public Functions ****************************************************/
extern elf_file_t *elf_maker_init();
extern elf_section_t *elf_maker_add_section(elf_file_t *elf_file, char *section_name);
extern elf_section_entry_t *elf_maker_add_section_entry(elf_section_t *section, void *data, size_t size);
extern void elf_maker_write(elf_file_t *elf_file, FILE *output);
extern void elf_maker_free(elf_file_t *elf_file);

/*********************************************** Private Functions ***************************************************/
extern void _elf_maker_section_free(elf_section_t *section);
extern void _elf_maker_section_entry_free(elf_section_entry_t *section);
extern void _elf_maker_prepare_for_writing(elf_file_t *elf_file);

#endif