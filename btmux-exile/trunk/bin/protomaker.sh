#!/bin/sh

INC=$1
shift
INCLUDES=$INC

for a in $*
do
  echo -n "Creating protos for $a.."
  NEWFILE_REAL=p.`basename $a .c`.h
  NEWFILE=${NEWFILE_REAL}.tmp
  if [ ! -f $NEWFILE_REAL -o $a -nt $NEWFILE_REAL ]
  then
    IFVAR=_`echo $NEWFILE_REAL | sed 's/\./_/g' | sed 's/\-/\_/g' | perl -p -e 'tr/[a-z]/[A-Z]/'`
    GENDATE=`date`
    cat <<*EOF* > $NEWFILE
/*
   $NEWFILE_REAL

   Automatically created by protomaker (C) 1998 Markus Stenberg (fingon@iki.fi)
   Protomaker is actually only a wrapper script for cproto, but well.. I like
   fancy headers and stuff :)
   */

/* Generated at $GENDATE from $a */
 
#ifndef $IFVAR
#define $IFVAR

*EOF*
    cproto  $INCLUDES $a 2>/dev/null >> $NEWFILE
    echo >> $NEWFILE
    echo "#endif /* $IFVAR */" >> $NEWFILE
    mv $NEWFILE $NEWFILE_REAL
  fi
  echo "Done."
done
