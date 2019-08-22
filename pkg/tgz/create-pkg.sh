#!/bin/sh

project="embedlog"
scp_server="pkgs@kurwik"
revision="1"

if [ ${#} -ne 3 ]
then
    echo "usage: ${0} <version> <arch> <host_os>"
    echo ""
    echo "where"
    echo "    <version>        git tag version"
    echo "    <arch>           target architecture"
    echo "    <host_os>        target os (slackware-14.2 etc)"
    exit 1
fi

git_version="${1}"
arch="${2}"
host_os="${3}"
workdir="/tmp/${project}-${arch}-${git_version}"

set -e

rm -rf "${workdir}"
mkdir "${workdir}"
cd "${workdir}"

wget "https://git.bofc.pl/${project}/snapshot/${project}-${git_version}.tar.gz"
tar xf "${project}-${git_version}.tar.gz"
cd "${project}-${git_version}"

version="$(grep "AC_INIT(" "configure.ac" | cut -f3 -d\[ | cut -f1 -d\])"
./autogen.sh
CFLAGS="-I/usr/bofc/include" LDFLAGS="-L/usr/bofc/lib" ./configure --prefix=/usr
LD_LIBRARY_PATH=/usr/bofc/lib make check

mkdir "${workdir}/root"
mkdir "${workdir}/root/install"
DESTDIR="${workdir}/root" make install

[ -f "pkg/tgz/doinst.sh" ] && cp "pkg/tgz/doinst.sh" "${workdir}/root/install"
cd "${workdir}/root"
find usr/share/man \( -name *.3 -or -name *.7 \) | xargs gzip
makepkg -l y -c n "${workdir}/${project}-${version}-${arch}-${revision}.tgz"
installpkg "${workdir}/${project}-${version}-${arch}-${revision}.tgz"

failed=0
gcc "${workdir}/${project}-${git_version}/pkg/test.c" -o "${workdir}/testprog" \
    -lembedlog || failed=1

if ldd "${workdir}/testprog" | grep "\/usr\/bofc"
then
    # sanity check to make sure test program uses system libraries
    # and not locally installed ones (which are used as build
    # dependencies for other programs

    echo "test prog uses libs from manually installed /usr/bofc \
        instead of system path!"
    failed=1
fi

"${workdir}/testprog" || failed=1

removepkg "${project}"

# run test prog again, but now fail if there is no error, testprog
# should fail as there is no library in te system any more
"${workdir}/testprog" && failed=1

if [ ${failed} -eq 1 ]
then
    exit 1
fi

if [ -n "${scp_server}" ]
then
    echo "copying data to ${scp_server}:${project}/${host_os}/${arch}"
    scp "${workdir}/${project}-${version}-${arch}-${revision}.tgz" \
        "${scp_server}:${project}/${host_os}/${arch}" || exit 1
fi
