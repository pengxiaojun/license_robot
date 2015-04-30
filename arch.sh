#!/bin/sh

ARCHDAT=$(dirname $0)/.arch.dat
if [ -f $ARCHDAT ]; then
	cat $ARCHDAT;
else
	ARCH_TEST=/tmp/arch.test.app

	echo '
#include <stdio.h>

int main()
{
	printf("%d", sizeof(void*));
}
' | gcc -x "c" - -o $ARCH_TEST
	ARCH_STR=`$ARCH_TEST`
	rm -f $ARCH_TEST

	if [ $ARCH_STR == "4" ]; then
		echo x86 | tee $ARCHDAT;
	else
		echo x64 | tee $ARCHDAT;
	fi;
fi
