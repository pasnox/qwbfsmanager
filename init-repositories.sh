#!/bin/bash

GIT=/usr/bin/git

if [ ! -f ${GIT} ]; then
    GIT=/opt/local/bin/git
fi

# Update external git repositories needed to build the project
"$GIT" submodule update --init --recursive
