#!/bin/sh
# © 2021 David Given.
# WordGrinder is licensed under the MIT open source license. See the COPYING
# file in this distribution for the full text.

set -e

output="$1"
shift
self="$1"
shift
objectify="$1"
shift

symbol="$(basename "$output" .h)"

out() {
	echo "$@" >>"$output"
}

echo >"$output"

count=0
for f in "$@"; do
	out
	out "/* This is $f */"
	python3 "$objectify" $f file_$count >>"$output"
	count=$(expr $count + 1)
done

out "const FileDescriptor $symbol[] = {"
count=0
for f in "$@"; do
	out "  { std::string((const char*) file_$count, sizeof(file_$count)), \"$f\" },"
	count=$(expr $count + 1)
done
out "  {}"
out "};"
