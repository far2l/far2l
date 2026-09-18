#!/bin/bash

git fetch origin pull/$1/head:pr$1
git merge pr$1
