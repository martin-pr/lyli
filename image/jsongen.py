#!/usr/bin/env python3

TYPE_MAP = {
	"bool": "bool",
	"int": "int",
	"float": "float",
	"string": "std::string"
}

class HdrGenVisitor:
	def __init__(self):
		self.level = 0

	def gen(self, str):
		indent = self.level * "\t"
		print(indent + str)

	def visitClass(self, cls):
		self.gen("")
		self.gen("class " + cls.name.title() + " {")

		self.level = self.level + 1
		for node in cls.members:
			node.accept(self)
		self.level = self.level - 1

		self.gen("};")
		self.gen(cls.name.title() + " get" + cls.name.title() + "() const;")
		self.gen("")

	def visitPrimitive(self, primitive):
		self.gen(TYPE_MAP[primitive.type] + " get" + primitive.name.title() + "() const;")

	def visitArray(self, primitive):
		self.gen(TYPE_MAP[primitive.type] + "* get" + primitive.name.title() + "() const;")
