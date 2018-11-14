#!/bin/sh

scp_server="pkgs@kurwik"
project="embedlog"

if [ ${#} -ne 3 ]
then
    echo "usage: ${0} <version> <arch> <host_os>"
    echo ""
    echo "where:"
    echo "    <version>         git tag version to build (without prefix v)"
    echo "    <arch>            target architecture"
    echo "    <host_os>         target os (debian9, debian8 etc)"
    echo ""
    echo "example"
    echo "      ${0} 1.0.0 i386 debian9"
    exit 1
fi

version="${1}"
arch="${2}"
host_os="${3}"

###
# preparing
#

rm -rf "/tmp/${project}-${version}"
mkdir "/tmp/${project}-${version}"

cd "/tmp/${project}-${version}"
git clone "https://git.kurwinet.pl/${project}"
cd "${project}"

git checkout "${version}" || exit 1

if [ ! -d "pkg/deb" ]
then
    echo "pkg/deb does not exist, cannot create debian pkg"
    exit 1
fi

version="$(grep "AC_INIT(" "configure.ac" | cut -f3 -d\[ | cut -f1 -d\])"
abi_version="$(echo ${version} | cut -f1 -d.)"

echo "version ${version}"
echo "abi version ${abi_version}"

###
# building package
#

codename="$(lsb_release -c | awk '{print $2}')"

cp -r "pkg/deb/" "debian"
sed -i "s/@{DATE}/$(date -R)/" "debian/changelog.template"
sed -i "s/@{VERSION}/${version}/" "debian/changelog.template"
sed -i "s/@{CODENAME}/${codename}/" "debian/changelog.template"
sed -i "s/@{ABI_VERSION}/${abi_version}/" "debian/control.template"

mv "debian/changelog.template" "debian/changelog"
mv "debian/control.template" "debian/control"

export CFLAGS="-I/usr/bofc/include"
export LDFLAGS="-L/usr/bofc/lib"
debuild -us -uc || exit 1

# unsed, so it these don't pollute gcc, when we built test program
unset CFLAGS
unset LDFLAGS

###
# verifying
#

cd ..

# debuild doesn't fail when lintial finds an error, so we need
# to check it manually, it doesn't take much time, so whatever

for d in *.deb
do
    echo "Running lintian on ${d}"
    lintian ${d} || exit 1
done

dpkg -i "lib${project}${abi_version}_${version}_${arch}.deb" || exit 1
dpkg -i "lib${project}-dev_${version}_${arch}.deb" || exit 1

failed=0
gcc ${project}/pkg/test.c -o testprog -lembedlog || failed=1

if ldd ./testprog | grep "\/usr\/bofc"
then
    # sanity check to make sure test program uses system libraries
    # and not locally installed ones (which are used as build
    # dependencies for other programs

    echo "test prog uses libs from manually installed /usr/bofc \
        instead of system path!"
    failed=1
fi

./testprog || failed=1

dpkg -r "lib${project}${abi_version}" "lib${project}-dev" || exit 1

if [ ${failed} -eq 1 ]
then
    exit 1
fi

if [ -n "${scp_server}" ]
then
    dbgsym_pkg="lib${project}${abi_version}-dbgsym_${version}_${arch}.deb"

    if [ ! -f "${dbgsym_pkg}" ]
    then
        # on some systems packages with debug symbols are created with
        # ddeb extension and not deb
        dbgsym_pkg="lib${project}${abi_version}-dbgsym_${version}_${arch}.ddeb"
    fi

    echo "copying data to ${scp_server}:${project}/${host_os}/${arch}"
    scp "lib${project}-dev_${version}_${arch}.deb" \
        "${dbgsym_pkg}" \
        "lib${project}${abi_version}_${version}_${arch}.deb" \
        "lib${project}_${version}.dsc" \
        "lib${project}_${version}.tar.xz" \
        "lib${project}_${version}_${arch}.build" \
        "lib${project}_${version}_${arch}.buildinfo" \
        "lib${project}_${version}_${arch}.changes" \
        "${scp_server}:${project}/${host_os}/${arch}" || exit 1
fi
