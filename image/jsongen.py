#!/usr/bin/env python3

level = 0

TYPE_MAP = {
	"bool": "bool",
	"int": "int",
	"float": "float",
	"string": "std::string"
}

def gen(str):
	global level
	indent = level * "\t"
	print(indent + str)

def beginClass(name, path):
	global level
	gen("")
	gen("class " + name.title() + " {")
	level = level + 1

def endClass(name, path):
	global level
	level = level - 1
	gen("};")
	gen(name.title() + " get" + name.title() + "() const;")
	gen("")

def genPrimitive(type, name, path):
	gen(TYPE_MAP[type] + " get" + name.title() + "() const;")
