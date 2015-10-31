#!/usr/bin/env python3

import re
import sys

import jsongen

TOKEN_TYPE = {}

def getName(address):
	'''Get name of an object from its address.

	This function returns the last part of an address and strips any non-alpha
	numeric characters (eg. if the part is an array)

	'''
	return re.match("\w+", address.split("/")[-1]).group(0)

def parseClass(file, type, tokens, address):
	name = getName(tokens[0])
	address = address + tokens[0]
	jsongen.beginClass(name, address)

	# parse lines within class until the closing brace is found
	line = file.readline()
	while line:
		if line.lstrip()[0] == "}":
			jsongen.endClass(name, address)
			return
		else:
			parseLine(file, line, address)
			line = file.readline()
	jsongen.endClass(name, address)

def parsePrimitive(file, type, tokens, address):
	jsongen.genPrimitive(type, getName(tokens[0]), address + tokens[0])

def parseLine(file, line, address):
	line = line.strip()
	if line and line[0] != "#":
		tokens = line.split()
		TOKEN_TYPE[tokens[0]](file, tokens[0], tokens[1:], address)

TOKEN_TYPE["class"] = parseClass
TOKEN_TYPE["bool"] = parsePrimitive
TOKEN_TYPE["int"] = parsePrimitive
TOKEN_TYPE["float"] = parsePrimitive
TOKEN_TYPE["string"] = parsePrimitive

if __name__ == '__main__':
	if len(sys.argv) < 2:
		print("missing argument")
		exit(1)
	file = open(sys.argv[1])

	# read the file line by line
	line = file.readline()
	while line:
		parseLine(file, line, "/")
		line = file.readline()
