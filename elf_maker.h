#ifndef elf_maker_h
#define elf_maker_h

/*************************************************** Includes ********************************************************/
#include <elf.h>
#include "SLL.h"
#include <stdio.h>
/********************************************* Constants Definitions *************************************************/
#define SECTION_NAME_MAX_LEN 15 /* maximum string length for a section name */

/*********************************************** Types Definitions ***************************************************/
/* elf and section headers - wrapers*/
typedef Elf32_Ehdr elf_header_t;
typedef Elf32_Shdr elf_section_header_t;

/* elf section struct */
typedef struct elf_section_t
{
  char section_name[SECTION_NAME_MAX_LEN + 1];
  elf_section_header_t section_header;
  void *section_data;
} elf_section_t;

/* elf file struct */
typedef struct elf_file_t
{
  elf_header_t file_header;
  SLL *sections_list;
} elf_file_t;

/*********************************************** Public Functions ****************************************************/
extern elf_file_t *elf_maker_init();
extern void elf_maker_free(elf_file_t *elf_file);
extern elf_section_t *elf_maker_add_section(elf_file_t *elf_file, char *section_name, void *section_data, size_t data_size);
extern void elf_maker_write(elf_file_t *elf_file, FILE *output);

/*********************************************** Private Functions ***************************************************/
extern void _elf_maker_section_free(elf_section_t *section);
extern void _elf_maker_prepare_for_writing(elf_file_t *elf_file);


#endif