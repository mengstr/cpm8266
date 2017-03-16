#!/bin/bash

INFILE=$1
WORKFILE=$1_work

SKEW=6
SECTORS=26

# BUILD THE SKEW/INTERLEAVE TRANSLATION TABLE
declare -a skewtab

  j=0
  for ((i=0; i < $SECTORS; i++)) {
    for ((k=0; k<$i; k++)) {
	if [ ${skewtab[$k]} -eq $j ];  then break; fi
    }
    if [[ $k -lt $i ]]; then j=$(( ($j + 1) % $SECTORS )); fi
    skewtab[$i]=$j
    j=$(( ($j + $SKEW) % $SECTORS ))
  }

#  echo "1 7 13 19 25 5 11 17 23 3 9 15 21 2 8 14 20 26 6 12 18 24 4 10 16 22"
#  for ((i=0; i < $SECTORS; i++)) {
#	x=$(( ${skewtab[$i]} + 1 ))
#  	echo -n "$x "
#  }
#  echo 
  	
#Get the file as hex into an array where each entry is a full 128 byte sector
IFS=$'\n' sector=($(od -t x1 -An -v -w128 $INFILE))


# The disk have 77 tracks, with 26 sectors each.
# The first two tracks are reserved for the boot sectors and are not 
# interleaved so they can be left as-is.
for TRK in {2..76}; do
  # copy all sectors in current track to a holding array
  for i in {0..25}; do 
    hold[$i]=${sector[$(($TRK*26+$i))]}
  done
  # Re-arrange the sectors from the holding array into the main sector array
  for i in {0..25}; do 
    sector[(($TRK*26+$i))]=${hold[${skewtab[$i]}]}
  done
done

# Convert the entire array from hex back to a binary file
echo ${sector[*]} | xxd -r -p > $WORKFILE
rm $INFILE
mv $WORKFILE $INFILE

