include $(top_srcdir)/embedlog-sources.mk
CFLAGS += -I$(top_srcdir)/include -I$(top_srcdir)/src
EXTRA_PROGRAMS = print_to_file \
	print_options \
	print_simple \
	print_memory \
	print_tty

print_to_file_SOURCES = print-to-file.c $(embedlog_sources)
print_options_SOURCES = print-options.c $(embedlog_sources)
print_simple_SOURCES = print-simple.c $(embedlog_sources)
print_memory_SOURCES = print-memory.c $(embedlog_sources)
print_tty_SOURCES = print-tty.c $(embedlog_sources)

examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)
