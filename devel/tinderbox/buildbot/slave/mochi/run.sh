#!/bin/sh
cd /home/verm/buildslave
exec env -i PATH="$PATH" buildbot start .
