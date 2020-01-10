#!/bin/bash

curl -O http://www.sociopatterns.org/wp-content/uploads/2018/12/tij_SFHH.dat_.gz
gunzip tij_SFHH.dat_.gz # tij_SFHH.dat_.gz -> tij_SFHH.dat_
cut -d ' ' -f "2 3" tij_SFHH.dat_ | tr ' ' '\n' | sort -g | uniq |\
    nl -n ln -s ' ' -v 0 > tij_SFHH.verts
cat tij_SFHH.dat_ |\
    sort -g -t ' ' -k 2 | join -1 2 -2 2 -o "1.1 2.1 1.3" - tij_SFHH.verts |\
    sort -g -t ' ' -k 3 | join -1 3 -2 2 -o "1.1 1.2 2.1" - tij_SFHH.verts |\
    sort -g -t ' ' -k 1 | awk '{print $2,$3,$1}' > tij_SFHH.edgelist

echo total $(cat tij_SFHH.verts | wc -l) vertices

rm tij_SFHH.verts tij_SFHH.dat_
