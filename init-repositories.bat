@echo off

REM Update external git repositories needed to build the project
git submodule update --init --recursive
