#!/bin/sh

set -ex

make

mkdir -p submissions

ruby solve_all.rb 2>&1 | tee /tmp/log

export TZ=UTC
d=`date +'%H%M%d'`

curl --user :$(cat api_token) -X POST -H "Content-Type: application/json" -d @out.json https://davar.icfpcontest.org/teams/196/solutions

cp /tmp/log submissions/$d.log
cp -r logs submissions/logs-$d
cp out.json submissions/$d.json
