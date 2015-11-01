#!/usr/bin/env python3

from io import StringIO
from string import Template

import re

import jsontemplates

TYPE_MAP = {
	"bool": "bool",
	"int": "int",
	"float": "float",
	"string": "std::string"
}

ACCESS_MAP = {
	"bool": "asBool()",
	"int": "asInt()",
	"float": "asFloat()",
	"string": "asString()"
}

class HdrGenVisitor:
	def __init__(self, classname):
		self.classname = classname
		self.level = 1
		self.code = StringIO()

	def gen(self, str=None):
		indent = self.level * "\t"
		if str:
			self.code.write(indent)
			self.code.write(str)
		self.code.write("\n")

	def visitClass(self, cls):
		clstitle = cls.name.title()

		self.gen()
		self.gen("class " + clstitle + " {")
		self.gen("public:")
		self.gen("\t" + clstitle + "() = default;")
		self.gen("\t" + clstitle + "(const " + clstitle + "& other) = default;")
		self.gen("\t" + clstitle + "(const ValuePtr &root);")
		self.gen()

		self.level = self.level + 1
		for node in cls.members:
			node.accept(self)
		self.level = self.level - 1

		self.gen("private:")
		self.gen("\tValuePtr m_root;")

		self.gen("};")
		self.gen(clstitle + " get" + clstitle + "() const;")

	def visitPrimitive(self, primitive):
		self.gen(TYPE_MAP[primitive.type] + " get" + primitive.name.title() + "() const;")

	def visitArray(self, primitive):
		self.gen(TYPE_MAP[primitive.type] + "* get" + primitive.name.title() + "() const;")

	def write(self):
		file = open(self.classname.lower() + ".h", "w")
		code = jsontemplates.TEMPLATE_HEADER.substitute(CLASS=self.classname, GENERATED=self.code.getvalue())
		file.write(code)
		file.close

class SrcGenVisitor:
	def __init__(self, classname):
		self.classname = classname
		self.path = [classname]
		self.code = StringIO()

	def gen(self, str=None):
		if str:
			self.code.write(str)
		self.code.write("\n")

	def genPath(self):
		return "::".join(self.path)

	def visitClass(self, cls):
		clstitle = cls.name.title()
		self.path.append(clstitle)

		# constructor
		self.gen(self.genPath() + "::" + clstitle + "(const ValuePtr &root) : m_root(root) {")
		self.gen()
		self.gen("}")
		self.gen()

		# member getters
		for node in cls.members:
			node.accept(self)

		# pop!
		self.path.pop()

		# class getter
		self.gen(self.genPath() + "::" + clstitle + " " + self.genPath() + "::get" + clstitle + "() const {")
		self.gen("\treturn " + clstitle + "(m_root);")
		self.gen("}")
		self.gen()



	def visitPrimitive(self, primitive):
		pathlist = filter(None, primitive.path.split('/'))
		pathaccess = ""
		for elem in pathlist:
			match = re.match("(.*)\[(.*)\]", elem)
			if match:
				pathaccess += '["' + match.group(1) + '"]'
				pathaccess += '[' + match.group(2) + ']'
			else:
				pathaccess += '["' + elem + '"]'

		self.gen(TYPE_MAP[primitive.type] + " " + self.genPath() + "::get" + primitive.name.title() + "() const {")
		self.gen("\treturn (*m_root)" + pathaccess + "." + ACCESS_MAP[primitive.type] + ";")
		self.gen("}")
		self.gen()


	def visitArray(self, primitive):
		pass
		# TODO
		#self.gen(TYPE_MAP[primitive.type] + "* get" + primitive.name.title() + "() const;")

	def write(self):
		file = open(self.classname.lower() + ".cpp", "w")
		code = jsontemplates.TEMPLATE_SOURCE.substitute(INCLUDE=self.classname.lower(), CLASS=self.classname, GENERATED=self.code.getvalue())
		file.write(code)
		file.close
