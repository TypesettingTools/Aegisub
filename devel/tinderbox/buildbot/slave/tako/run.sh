#!/bin/sh
cd /usr/home/verm/buildslave
exec env -i PATH="$PATH" buildbot start .
