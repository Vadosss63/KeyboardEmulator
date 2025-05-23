#!/usr/bin/env bash

RETURN_CODE_SUCCESS=0
RETURN_CODE_ERROR=1

FILE_LIST="$(find "$@" | grep -E ".*\.(ino|cpp|hpp|hh|h|c)$")"

IFS=$'\n' read -r -d '' -a FILE_LIST_ARRAY <<< "$FILE_LIST"

num_files="${#FILE_LIST_ARRAY[@]}"
echo -e "$num_files files found to format:"
if [ "$num_files" -eq 0 ]; then
    echo "Nothing to do."
    exit $RETURN_CODE_SUCCESS
fi

# print the list of all files
for i in "${!FILE_LIST_ARRAY[@]}"; do
    file="${FILE_LIST_ARRAY["$i"]}"
    printf "  %2i: %s\n" $((i + 1)) "$file"
done
echo ""

#format_files="false"
#read -p "Do you wish to auto-format all of these files [y/N] " user_response
#case "$user_response" in
#    [Yy]* ) format_files="true"
#esac
format_files="true"

if [ "$format_files" = "false" ]; then
    echo "Aborting."
    exit $RETURN_CODE_SUCCESS
fi

clang-format --verbose -i --style=file "${FILE_LIST_ARRAY[@]}"
