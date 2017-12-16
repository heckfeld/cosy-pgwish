#!/bin/bash

#####
##### Variables
#####
PGHOST=${PGHOST:-134.94.224.10}
DATABASE=calib
TYP=
UNIT=

#####
##### Functions
#####

dopsql () {
    what="rtrim(name,' '), rtrim(typ,' '), rtrim(einheit,' '), xanfang, xende, rtrim(funktion,' '), a0, a1, a2, a3, a4, a5"
    command="SELECT $what from calib"
    if test x"$1" != x
    then
	command="$command where typ ~ '$1'"
	if test x"$2" != x
	then
	    command="$command and einheit ~ '$2'"
	fi
    else
	if test x"$2" != x
	then
	    command="$command where einheit ~ '$2'"
	fi
    fi
    command="$command ;"
    psql --tuples-only --host=$PGHOST --dbname=$DATABASE << EOF | tr "|" ":" | tr -d " "
$command
\q
EOF
}

#####
##### Options
#####

TEMP=`getopt -o at:u: -n 'calib.sh' -- "$@"`

if [ $? != 0 ] ; then echo "Terminating..." >&2 ; exit 1 ; fi

# Note the quotes around `$TEMP': they are essential!
eval set -- "$TEMP"

while true ; do
        case "$1" in
                -a) echo "Option a" ; break ; shift ;;
                -t) echo "Option type" ; TYP=$2 ; shift 2 ;;
                -u) echo "Option unit" ; UNIT=$2 ; shift 2 ;;
                --) shift ; break ;;
                *) echo "Internal error!" ; exit 1 ;;
        esac
done

dopsql $TYP $UNIT
