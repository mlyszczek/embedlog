#!/bin/sh

project='embedlog'
out='www/downloads.html'
remote="https://distfiles.bofc.pl/${project}"
gpg="https://distfiles.bofc.pl/bofc-signing.pub.gpg"
asc="https://distfiles.bofc.pl/bofc-signing.pub.asc"

get_files_from_remote()
{
    remote="${1}"

    curl "${remote}/" -A "${project}-generator" 2>/dev/null | grep "a href=" | \
        grep -v 'a href=".."' | cut -f2 -d\" | cut -f1 -d/
}

# extract links to files
links="$(curl "${remote}/" -A "${project}-generator" 2>/dev/null \
    | grep "${project}-[0-9]*\.[0-9]*\.[0-9]*\." | sort -r)"


cat << EOF
=========
downloads
=========
Below you can find released source files. (s) right of package name is a gpg
signature. You can download gpg file or armored asc file to verify files.
You can also look for key on public keyservers, fingerprint is::

    63D0 C3DB 42AF 3B4F CF6E 7880 E84A 7E61 C785 0C62

You can download key directly from keyserver with::

    gpg --recv-keys 63D0C3DB42AF3B4FCF6E7880E84A7E61C7850C62

Then you can verify downloaded image with command::

    gpg --verify <sig-file> <package-file>

All files (including md5, sha256 and sha512 for all files) can also be
downloaded from: https://distfiles.bofc.pl/embedlog

git
---
::

   git clone git@git.bofc.pl:embedlog
   git clone git://git.bofc.pl/embedlog
   git clone http://git.bofc.pl/embedlog

tarballs (source code)
----------------------
.. parsed-literal::

EOF

files="$(get_files_from_remote "${remote}/" | \
    grep "${project}-[0-9]*\.[0-9]*\.[0-9][\.-]\(r[0-9]\)\?")"
versions="$(echo "${files}" | tr ' ' '\n' | rev | \
    cut -f1 -d- | rev | cut -f1-3 -d. | sort -Vur)"

for v in ${versions}; do
	tgz=${remote}/${project}-${v}.tar.gz
	tgz_s=${remote}/${project}-${v}.tar.gz.sig
	tgz_h=${remote}/${project}-${v}.tar.gz.sha1
	tbz=${remote}/${project}-${v}.tar.bz2
	tbz_s=${remote}/${project}-${v}.tar.bz2.sig
	tbz_h=${remote}/${project}-${v}.tar.bz2.sha1
	txz=${remote}/${project}-${v}.tar.xz
	txz_s=${remote}/${project}-${v}.tar.xz.sig
	txz_h=${remote}/${project}-${v}.tar.xz.sha1

	printf "   %-10s%s\\ (%s\\|\\ %s)  %s\\ (%s\\ \\|\\ %s)  %s\\ (%s\\|\\ %s)\n" "${v}" \
		"\`tar.gz <$tgz>\`__" \
		"\`s <$tgz_s>\`__" \
		"\`sha1 <$tgz_h>\`__" \
		"\`tar.bz2 <$tbz>\`__" \
		"\`s <$tbz_s>\`__" \
		"\`sha1 <$tbz_h>\`__" \
		"\`tar.xz <$txz>\`__" \
		"\`s <$txz_s>\`__" \
		"\`sha1 <$txz_h>\`__"

done

failed=0
exit 0

for l in $(lynx -listonly -nonumbers -dump "${out}" | grep "https://distfiles")
do
    echo -n "checking ${l}... "
    curl -sSfl -A "${project}-generator" "${l}" >/dev/null

    if [ ${?} -eq 0 ]
    then
        echo "ok"
        continue
    fi

    failed=1
done

exit ${failed}
