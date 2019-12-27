#!/bin/bash

sort -t ' ' -k 2 -g networks.list | head -n3 | grep -v "^#" | while read net order size direction
do
    edgelist="edgelists/${net}.edgelist"
    dybccache="${net}.dybccache"
    diflag="$(echo $direction | sed s/undirected// | sed s/directed/-d/)"

    echo "Building cache for $net (order: $order, size: $size, diflag: $diflag)"
    ../../release/exp-calculate-cache \
        $diflag $edgelist $dybccache
    echo "Finished building cache ($(stat --printf='%s' $dybccache) bytes)"

    echo "Running experiment for $net"
    echo "$diflag -c $dybccache $edgelist" |\
        join -j99 - queries.list |\
        join -j99 seeds.list - |\
        xargs -L1 ../../release/exp-compare-performance >\
              "${net}.csv"
    rm $dybccache

done
