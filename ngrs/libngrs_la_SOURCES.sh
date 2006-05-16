#! /bin/sh

main()
{
	cd $(dirname $0) &&
	stanza=$(basename $0 .sh) &&
	out=$stanza.am
	echo "# this file was autogenerated by $0 ; do not edit" > $out &&
	for x in $(find src/ngrs \( -type d -name todo -prune \) -or \( -name \*.cpp -print \))
	do
		echo $stanza += $x >> $out
	done
} &&

main "$0"
