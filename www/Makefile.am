EXTRA_DIST = custom.css footer.in header.in index.in index.md

www:
	kursg -o out
	./post-process.sh
	chmod a+rX out -R

make clean:
	$(RM) -r out
	$(RM) -r manuals
