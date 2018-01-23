ACLOCAL_AMFLAGS=-I m4

SUBDIRS = include src tst man examples www
EXTRA_DIST = tap-driver man2html.sh

www:
	./man2html.sh
	make www -C www

.PHONY: www
