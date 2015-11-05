#!/usr/bin/env python3

import os
import re
import sys

from pycppjson import jsonast
from pycppjson import jsongen

def getName(address):
	'''Get name of an object from its address.

	This function returns the last part of an address and strips any non-alpha
	numeric characters (eg. if the part is an array)

	'''
	return re.match("\w+", address.split('/')[-1]).group(0)

def parseClass(file, type, tokens, address):
	name = getName(tokens[0])
	address = address + '/' + tokens[0]

	cls = jsonast.Class(name)

	# parse lines within class until the closing brace is found
	line = file.readline()
	while line:
		if line.lstrip()[0] == "}":
			return cls
		else:
			node = parseLine(file, line, address)
			if node:
				cls.addMember(node)
			line = file.readline()
	return cls

def parsePrimitive(file, type, tokens, address):
	return jsonast.Primitive(type, getName(tokens[0]), address + '/' + tokens[0])

def parseArray(file, type, tokens, address):
	match = re.match("(\w+)\[(.*)\]", type)
	primitive = match.group(1)
	size = match.group(2)
	return jsonast.Array(primitive, size, getName(tokens[0]), address + '/' + tokens[0])

def parseLine(file, line, address):
	line = line.strip()
	if line and line[0] != "#":
		tokens = line.split()

		if tokens[0] == "class":
			return parseClass(file, tokens[0], tokens[1:], address)
		elif tokens[0][-1] == "]":
			return parseArray(file, tokens[0], tokens[1:], address)
		else:
			return parsePrimitive(file, tokens[0], tokens[1:], address)

	return None

if __name__ == '__main__':
	if len(sys.argv) < 3:
		print('missing argument')
		print("Usage:")
		print('\t jsonparser.py "Fully::Qualified::Class" file')
		exit(1)

	# get command line arguments
	quialifiedname = sys.argv[1].split("::")
	filename = sys.argv[2]

	# create json root and read the file line by line
	root = jsonast.Root()
	file = open(sys.argv[2])
	line = file.readline()
	while line:
		node = parseLine(file, line, "/")
		if node:
			root.addNode(node)
		line = file.readline()

	# generate output
	filebase = os.path.splitext(os.path.basename(filename))[0]
	hdrgen = jsongen.HdrGenVisitor(quialifiedname, filebase)
	root.accept(hdrgen)
	hdrgen.write()
	srcgen = jsongen.SrcGenVisitor(quialifiedname, filebase)
	root.accept(srcgen)
	srcgen.write()
