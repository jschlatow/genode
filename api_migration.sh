#!/bin/bash

if [ $# -ge 1 ]; then
	DIR=$1
else
	DIR="."
fi

function open {
	# strip line number and function name
	filename=${1%%:*}
	# get line number
	line=${1#*:}
	line=${line%%:*}

	vim +$line $filename
}

function iterate_and_open {
	for file in $1; do
		read -r -p "Edit '$file'? [y/n] " response
		if [[ $response =~ ^[yY]$ ]]; then
			open $file
		fi
	done
}

# first to safe search-and-replace operations
find $DIR -type f -exec \
	sed -i 's#os/server.h#base/component.h#g' {} +;

# first to safe search-and-replace operations
find $DIR -type f -exec \
	sed -i 's#Server::Entrypoint#Genode::Entrypoint#g' {} +;

# first to safe search-and-replace operations
find $DIR -type f -exec \
	sed -i 's#Signal_rpc_member#Signal_handler#g' {} +;

# first to safe search-and-replace operations
find $DIR -name target.* -type f -exec \
	sed -i 's# server##g' {} +;

fixlist=$(grep -rn "Thread<" $DIR | awk '{ print $1 }')
if [ -n "$fixlist" ]; then
	echo "Please replace Thread<X> with either Thread_deprecated<X> or Thread (gets stack size via constructor), e.g."
	echo "  Thread foobar(Weight::DEFAULT_WEIGHT, \"name\", X)"
	iterate_and_open "$fixlist"
fi

fixlist=$(grep -rn "namespace Server" $DIR | awk '{ print $1 }')
if [ -n "$fixlist" ]; then
	echo "Replace 'Server' namespace by 'Component'. Also remove Server::name() function, e.g.:"
	iterate_and_open "$fixlist"
fi

fixlist=$(grep -rn "Genode::env()" $DIR | awk '{ print $1 }')
if [ -n "$fixlist" ]; then
	echo "Replace Genode::env()"
	iterate_and_open "$fixlist"
fi

fixlist=$(grep -rn "Genode::config()" $DIR | awk '{ print $1 }')
if [ -n "$fixlist" ]; then
	echo "Replace Genode::config() by Attached_rom_dataspace, e.g.:"
	echo "  Genode::Attached_rom_dataspace config(env, \"config\");"
	iterate_and_open "$fixlist"
fi

fixlist=$(grep -rn "base/printf.h" $DIR | awk '{ print $1 }')
if [ -n "$fixlist" ]; then
	echo "Remove base/printf.h"
	iterate_and_open "$fixlist"
fi

fixlist=$(grep -rn "base/console.h" $DIR | awk '{ print $1 }')
if [ -n "$fixlist" ]; then
	echo "Remove base/console.h"
	iterate_and_open "$fixlist"
fi

fixlist=$(grep -rn "base/snprintf.h" $DIR | awk '{ print $1 }')
if [ -n "$fixlist" ]; then
	echo "Remove base/snprintf.h"
	iterate_and_open "$fixlist"
fi

fixlist=$(grep -rn "PLOG" $DIR | awk '{ print $1 }')
if [ -n "$fixlist" ]; then
	echo "Replace PLOG"
	iterate_and_open "$fixlist"
fi

fixlist=$(grep -rn "PDBG" $DIR | awk '{ print $1 }')
if [ -n "$fixlist" ]; then
	echo "Replace PDBG"
	iterate_and_open "$fixlist"
fi

fixlist=$(grep -rn "PERR" $DIR | awk '{ print $1 }')
if [ -n "$fixlist" ]; then
	echo "Replace PERR"
	iterate_and_open "$fixlist"
fi

fixlist=$(grep -rn "PWRN" $DIR | awk '{ print $1 }')
if [ -n "$fixlist" ]; then
	echo "Replace PWRN"
	iterate_and_open "$fixlist"
fi

