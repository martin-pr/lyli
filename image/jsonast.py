#!/usr/bin/env python3

class Root:
	def __init__(self):
		self.nodes = []

	def addNode(self, node):
		self.nodes.append(node)

	def accept(self, visitor):
		for node in self.nodes:
			node.accept(visitor)

class Class:
	def __init__(self, name):
		self.name = name
		self.members = []

	def addMember(self, member):
		self.members.append(member);

	def accept(self, visitor):
		visitor.visitClass(self)

class Primitive:
	def __init__(self, type, name, path):
		self.type = type
		self.name = name
		self.path = path

	def accept(self, visitor):
		visitor.visitPrimitive(self)

class Array:
	def __init__(self, type, size, name, path):
		self.type = type
		self.size = size
		self.name = name
		self.path = path

	def accept(self, visitor):
		visitor.visitArray(self)
