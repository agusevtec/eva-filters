#!/bin/bash

tools/from-git.sh "https://github.com/agusevtec/eva-core-sk.git" || exit 1
tools/from-git.sh "https://github.com/bxparks/EpoxyDuino.git" || exit 1
tools/from-git.sh "https://github.com/bxparks/AUnit.git" || exit 1

echo "All repositories cloned successfully!"
