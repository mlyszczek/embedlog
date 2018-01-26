ACLOCAL_AMFLAGS=-I m4

SUBDIRS = include src tst man examples www
EXTRA_DIST = man2html.sh

www:
	./gen-download-page.sh
	./man2html.sh
	make www -C www

.PHONY: www
