#include "elf_maker.h"
#include "elf_maker_special_sections.h"
#include <string.h>
elf_section_t *add_null_section(elf_file_t *elf_file)
{
  if (!elf_file)
    return NULL;
  elf_section_t *null_section = elf_maker_add_section(elf_file, ".null");
  return null_section;
}

elf_section_t *add_strings_section(elf_file_t *elf_file)
{
  if (!elf_file)
    return NULL;

  SLLInfo *sections_info = elf_file->sections->data;

  elf_file->header.e_shstrndx = sections_info->size; /* tell elf file about the index of the strings section */
  elf_section_t *strings_section = elf_maker_add_section(elf_file, ".shstrtab");
  strings_section->header.sh_type = SHT_STRTAB;
  elf_maker_add_section_entry(strings_section, "\0", 1); /* null started */
  return strings_section;
}

elf_section_t *add_symbol_table_section(elf_file_t *elf_file)
{
  if (!elf_file)
    return NULL;

  elf_section_t *symbol_table_section = elf_maker_add_section(elf_file, ".symtab");
  symbol_table_section->header.sh_type = SHT_SYMTAB;
  symbol_table_section->header.sh_entsize = sizeof(elf_symbol_table_entry_t);
  return symbol_table_section;
}

elf_section_t *add_symbol_table_strings_section(elf_file_t *elf_file, elf_section_t *symbol_table_section)
{
  if (!elf_file || !symbol_table_section)
    return NULL;

  SLLInfo *sections_info = elf_file->sections->data;
  symbol_table_section->header.sh_link = sections_info->size; /* tell symbol_table_section about the index of its strings section */
  elf_section_t *symbol_table_strings_section = elf_maker_add_section(elf_file, ".strtab");
  symbol_table_strings_section->header.sh_type = SHT_STRTAB;
  elf_maker_add_section_entry(symbol_table_strings_section, "\0", 1); /* null started */
  return symbol_table_strings_section;
}

int add_symbol_table_entry(elf_section_t *symbol_table_section, elf_section_t *symbol_table_strings_section,
                           elf_symbol_table_entry_t *entry, char *name)
{
  if (!symbol_table_section || !symbol_table_strings_section || !entry)
    return 0;

  entry->st_name = symbol_table_strings_section->header.sh_size;

  if (!elf_maker_add_section_entry(symbol_table_section, entry, sizeof(elf_symbol_table_entry_t)))
  {
    return 0;
  }

  if (!name || !elf_maker_add_section_entry(symbol_table_strings_section, name, strlen(name) + 1))
  {
    entry->st_name = 0; //if adding name entry failed, then set it back to '\0'
  }

  return 1;
}

elf_section_t *add_text_section(elf_file_t *elf_file, elf_section_t *symbol_table_section, elf_section_t *symbol_table_strings_section)
{
  if (!elf_file || !symbol_table_section || !symbol_table_strings_section)
    return NULL;

  SLLInfo *sections_info = elf_file->sections->data;

  /* add section */
  elf_section_t *text_section = elf_maker_add_section(elf_file, ".text");
  text_section->header.sh_type = SHT_PROGBITS;
  text_section->header.sh_flags = SHF_ALLOC | SHF_EXECINSTR;

  /* add  symbol table entry*/
  elf_symbol_table_entry_t sym_entry;
  memset(&sym_entry, 0, sizeof(elf_symbol_table_entry_t));
  sym_entry.st_shndx = sections_info->size - 1; /*index of text section*/
  sym_entry.st_info = STB_GLOBAL << 4;          /*TODO(fix this using elf macros):only elf32bit ELF32_ST_BIND(STB_GLOBAL);*/
  add_symbol_table_entry(symbol_table_section, symbol_table_strings_section, &sym_entry, "_start");

  return text_section;
}

void finalize_strings_section(elf_file_t *elf_file, elf_section_t *strings_section)
{
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
}