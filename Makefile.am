include Makefile.am.coverage
ACLOCAL_AMFLAGS=-I m4

SUBDIRS = include src tst man examples www
EXTRA_DIST = man2html.sh gen-download-page.sh readme.md tap-driver.sh

www:
	./gen-download-page.sh
	./man2html.sh
	make www -C www

analyze:
	make analyze -C src

examples: all
	make examples -C examples

.PHONY: www examples analyze
