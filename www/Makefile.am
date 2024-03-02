# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?= -j auto
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = .
BUILDDIR      = _build

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
html : Makefile
	$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

man: Makefile
	$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
	@echo fix broken tables
	sed -i 's/^center;$$/box center;/' $(BUILDDIR)/man/*

# sphinx-build clean does not remove _build dir
clean-local:
	rm -rf _build

man_MANS = \
	_build/man/el_cleanup.3 \
	_build/man/el_destroy.3 \
	_build/man/el_disable_output.3 \
	_build/man/el_enable_colors.3 \
	_build/man/el_enable_file_log.3 \
	_build/man/el_enable_output.3 \
	_build/man/el_enable_thread_safe.3 \
	_build/man/el_flush.3 \
	_build/man/el_init.3 \
	_build/man/el_new.3 \
	_build/man/el_ocleanup.3 \
	_build/man/el_odestroy.3 \
	_build/man/el_odisable_output.3 \
	_build/man/el_oenable_colors.3 \
	_build/man/el_oenable_file_log.3 \
	_build/man/el_oenable_output.3 \
	_build/man/el_oenable_thread_safe.3 \
	_build/man/el_oflush.3 \
	_build/man/el_oinit.3 \
	_build/man/el_ooption.3 \
	_build/man/el_opbinary.3 \
	_build/man/el_operror.3 \
	_build/man/el_opmemory.3 \
	_build/man/el_opmemory_table.3 \
	_build/man/el_oprint.3 \
	_build/man/el_oprint_extra_info.3 \
	_build/man/el_option.3 \
	_build/man/el_oputb.3 \
	_build/man/el_oputs.3 \
	_build/man/el_oset_custom_put.3 \
	_build/man/el_oset_file_sync.3 \
	_build/man/el_oset_log_level.3 \
	_build/man/el_oset_prefix.3 \
	_build/man/el_oset_timestamp.3 \
	_build/man/el_oset_tty_dev.3 \
	_build/man/el_overview.7 \
	_build/man/el_ovprint.3 \
	_build/man/el_pbinary.3 \
	_build/man/el_perror.3 \
	_build/man/el_pmemory.3 \
	_build/man/el_pmemory_table.3 \
	_build/man/el_print.3 \
	_build/man/el_print_extra_info.3 \
	_build/man/el_putb.3 \
	_build/man/el_puts.3 \
	_build/man/el_set_custom_put.3 \
	_build/man/el_set_file_sync.3 \
	_build/man/el_set_log_level.3 \
	_build/man/el_set_prefix.3 \
	_build/man/el_set_timestamp.3 \
	_build/man/el_set_tty_dev.3 \
	_build/man/el_vprint.3

EXTRA_DIST = $(man_MANS)

$(man_MANS) &:
	@$(SPHINXBUILD) -M man "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
	@echo fix broken tables
	sed -i 's/^center;$$/box center;/' $(BUILDDIR)/man/*
