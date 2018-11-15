ACLOCAL_AMFLAGS=-I m4

SUBDIRS = include src tst man examples www
EXTRA_DIST = man2html.sh gen-download-page.sh readme.md

www:
	./gen-download-page.sh
	./man2html.sh
	make www -C www

analyze:
	make analyze -C src

examples:
	make examples -C examples

.PHONY: examples www