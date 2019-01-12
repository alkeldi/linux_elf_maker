#ifndef elf_maker_special_sections_h
#define elf_maker_special_sections_h

/*************************************************** Includes ********************************************************/
#include "elf_maker.h"

/*********************************************** Public Functions ****************************************************/
extern elf_section_t *add_null_section(elf_file_t *elf_file);
extern elf_section_t *add_strings_section(elf_file_t *elf_file);
extern elf_section_t *add_symbol_table_section(elf_file_t *elf_file);
extern elf_section_t *add_symbol_table_strings_section(elf_file_t *elf_file, elf_section_t *symbol_table_section);
extern int add_symbol_table_entry(elf_section_t *symbol_table_section, elf_section_t *symbol_table_strings_section,
                           elf_symbol_table_entry_t *entry, char *name);
extern elf_section_t *add_text_section(elf_file_t *elf_file, elf_section_t *symbol_table_section, elf_section_t *symbol_table_strings_section);
extern void finalize_strings_section(elf_file_t *elf_file, elf_section_t *strings_section);

#endif