#!/usr/bin/env bash

# This is a script to automate the checking of Orion tests + examples for memory leaks with Valgrind.
# It takes one argument; the executable to test.

# Since there are some detected memory issues (memory leaks, bugs in Valgrind/libc, etc) that we can't do
# anything about, we just suppress them instead.
# NOTE! The suppressions that work for me don't work for everyone (in face, they probably ONLY work for
# me) as everyone's machine is different and the specified things could also be installed in different
# places, etc. So if they don't work for you, feel free to add suppressions to the existing files, or if
# there are any unrelated issues that Valgrind whines about, then create new .supp files for them.

# requires a binary file to be executed
if [ -z "${1}" ]
then
    echo -e "\e[1mvalgrind_check.sh: \e[31;1mfatal error: \e[0mno input file"
    echo -e "script terminated."
    exit
fi

TEST_EXEC="$(readlink -f "${1}")"
VALGRIND_CMD="valgrind"
PROJECT_PATH="$(dirname -- "$(dirname -- "$(readlink -f "${BASH_SOURCE}")")")"
SUPPRESSION_FILES=(
    "${PROJECT_PATH}/valgrind/valgrind_amd64.supp"
    "${PROJECT_PATH}/valgrind/valgrind_glfw.supp"
    "${PROJECT_PATH}/valgrind/valgrind_radeon.supp"
)

# write the command (arguments)
VALGRIND_CMD+=" --leak-check=full --track-origins=yes -s"

# add suppression files to command
for f in ${SUPPRESSION_FILES[@]}
do
    VALGRIND_CMD+=" --suppressions=${f}"
done

VALGRIND_CMD+=" ${TEST_EXEC}"

# clear to reduce confusion with previous output
# + display command
clear
echo -e "-----------------------------------------------"
echo -e "Running command:\n${VALGRIND_CMD}"
echo -e "-----------------------------------------------\n"

# execute command
${VALGRIND_CMD}
