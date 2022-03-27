# This file is a part of the Kithare programming language source code.
# The source code for Kithare programming language is distributed under the MIT
# license.
# Copyright (C) 2022 Kithare Organization
# 
# This is a stub makefile kept for user convenience. Kithare relies on a python 
# script to build Kithare. This makefile merely calls the Python builder, so 
# Python needs to be installed for this makefile to work

ifeq ($(OS),Windows_NT)
	PYTHON = py -3
else
	PYTHON = python3
endif

make:
	${PYTHON} build.py

test: make
	${PYTHON} build.py --make test

clean:
	${PYTHON} build.py --clean all

debug:
	${PYTHON} build.py --make debug

installer:
	${PYTHON} build.py --make installer
