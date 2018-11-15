EXTRA_PROGRAMS = print_to_file print_options print_simple print_memory
LDADD = -lembedlog

print_to_file_SOURCES = print-to-file.c
print_options_SOURCES = print-options.c
print_simple_SOURCES = print-simple.c
print_memory_SOURCES = print-memory.c

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)
