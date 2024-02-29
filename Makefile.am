include Makefile.am.coverage
ACLOCAL_AMFLAGS=-I m4

SUBDIRS = include src tst www examples
EXTRA_DIST = gen-download-page.sh readme.rst tap-driver.sh

www:
	./gen-download-page.sh > www/downloads.rst
	make -C www html

analyze:
	make analyze -C src

examples: all
	make examples -C examples

.PHONY: www examples analyze
