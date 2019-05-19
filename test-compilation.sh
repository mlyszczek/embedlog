#!/bin/bash

## ==========================================================================
#                               _         __
#                              (_)____   / /_   _____
#                             / // __ \ / __ \ / ___/
#                            / // /_/ // /_/ /(__  )
#                         __/ / \____//_.___//____/
#                        /___/
## ==========================================================================


prepare()
{
    project="${1}"
    slot="${2}"
    project_dir="${3}"

    # clone
    if ! git clone "${project_dir}" "${project}-${slot}"
    then
        echo "couldn't clone, sorry"
        exit 1
    fi

    # prepare directory for distcheck, we need to at least once call autogen
    # and configure (doesn't matter with what parameters

    cd "${project}-${slot}"
    ./autogen.sh
    ./configure
}

build()
{
    opts="${1}"
    project="${2}"
    slot="${3}"
    project_dir="${4}"
    workdir="${5}"
    seq="${6}"

    cd "${project}-${slot}"
    export AM_DISTCHECK_CONFIGURE_FLAGS="${opts}"
    make distcheck
    ret=${?}

    if [ ${ret} -eq 0 ]
    then
        printf "[ok]  %8d    ${opts}\n" ${seq} >> \
            "${project_dir}/compilation-test-results"

        # also, if test was ok, remove stdout from workdir - stdout takes
        # about 4megs of data, and with 60k possible compilation flags
        # it will take 240GB of data! And this number will only go up
        # in the future releses.

        unlink "${workdir}/result/${seq}/stdout"
    else
        printf "[nok] %8d    ${opts}\n" ${seq} >> \
            "${project_dir}/compilation-test-results"
    fi
}


## ==========================================================================
#                                                      __   _
#       ____   _____ ___   ____   ____ _ _____ ____ _ / /_ (_)____   ____
#      / __ \ / ___// _ \ / __ \ / __ `// ___// __ `// __// // __ \ / __ \
#     / /_/ // /   /  __// /_/ // /_/ // /   / /_/ // /_ / // /_/ // / / /
#    / .___//_/    \___// .___/ \__,_//_/    \__,_/ \__//_/ \____//_/ /_/
#   /_/                /_/
## ==========================================================================


workdir="/tmp/parallel-test-compilation"
combination_file="${workdir}/combinations"
project="embedlog"
optfile="test-compilation-options"
project_dir="$(dirname "${0}")"
cd "${project_dir}"
project_dir="$(pwd)"

export -f build
export -f prepare

if [ ${#} -ne 1 ]
then
    echo "usage ${0} <number-of-jobs>"
    exit 1
fi

num_jobs=${1}

# some sanity checks

if [ ${num_jobs} -ne ${num_jobs} ]
then
    echo "num jobs is garbage: ${num_jobs}"
    exit 1
fi

if [ ! -f "${optfile}" ]
then
    echo "file with options doesn't exist"
    exit 1
fi


## ==========================================================================
#                                __                __
#                         _____ / /_ ____ _ _____ / /_
#                        / ___// __// __ `// ___// __/
#                       (__  )/ /_ / /_/ // /   / /_
#                      /____/ \__/ \__,_//_/    \__/
#
## ==========================================================================


mapfile -t options < "${optfile}"

# distcheck (when failed) leave readonly files which cannot be deleted
chmod u+rwx -R "${workdir}"
rm -rf "${workdir}"
mkdir "${workdir}"
cd "${workdir}"
pwd

# generate all combination of build options

iterations=$((2**${#options[@]} - 1))
echo -n "" > "${combination_file}"
for i in $(seq 0 1 $iterations)
do
    opts=
    flags=`echo "obase=2;${i}" | bc | rev`
    for j in `seq 1 1 ${#flags}`
    do
        if [ "${flags:j-1:1}" == "1" ]
        then
            opts+="${options[j - 1]} "
        fi
    done
    echo "${opts}" >> "${combination_file}"
    echo -ne "\rgenerating combinations ${i}/${iterations}"
done

# remove traling spaces from our combination files
sed -i 's/[ ]*$//' "${combination_file}"

echo ""

# ok, so first we need to clone as many directories as many jobs in parallel
# will be run, it's because of the make distcheck that has harcoded values and
# it won't work in parallel in same directory as another distcheck

slots=
for i in $(seq 1 1 ${num_jobs})
do
    slots+="${i} "
done

# run preparation
parallel --output-as-files --bar --results "${workdir}" \
    --halt-on-error now,fail=1 --jobs ${num_jobs} \
    prepare ::: "${project}" ::: ${slots} ::: "${project_dir}"

# and run distcheck tests, will take a loooong time
mkdir -p "${workdir}/result"
cat ${combination_file} | parallel --output-as-files --bar \
    --results "${workdir}/result/{#}/" --halt-on-error now,fail=1 \
    --jobs ${num_jobs} \
    build {} "${project}" {%} "${project_dir}" "${workdir}" "{#}"
