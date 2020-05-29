CFLAGS += -I$(top_builddir)/include -I$(top_srcdir)/src
VLDFLAGS =
VLDADD = $(top_builddir)/src/libembedlog.la

EXTRA_PROGRAMS = print_to_file \
	print_options \
	print_simple \
	print_memory \
	print_tty

print_to_file_SOURCES = print-to-file.c
print_options_SOURCES = print-options.c
print_simple_SOURCES = print-simple.c
print_memory_SOURCES = print-memory.c
print_tty_SOURCES = print-tty.c

print_to_file_LDFLAGS = $(VLDFLAGS)
print_options_LDFLAGS = $(VLDFLAGS)
print_simple_LDFLAGS = $(VLDFLAGS)
print_memory_LDFLAGS = $(VLDFLAGS)
print_tty_LDFLAGS = $(VLDFLAGS)

print_to_file_LDADD = $(VLDADD)
print_options_LDADD = $(VLDADD)
print_simple_LDADD = $(VLDADD)
print_memory_LDADD = $(VLDADD)
print_tty_LDADD = $(VLDADD)

.PHONY: examples
examples: $(EXTRA_PROGRAMS)

clean-local:
	rm -f $(EXTRA_PROGRAMS)
