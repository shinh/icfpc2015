#!/bin/sh

set -ex

mkdir -p submissions

curl --user :$(cat api_token) -X POST -H "Content-Type: application/json" -d @out.json https://davar.icfpcontest.org/teams/196/solutions

export TZ=UTC
cp out.json submissions/`date +'%H%M%d'`.json
