
# -*- coding: utf-8 -*-

import threading

class atom:
	def __init__(self, init_value = 0):
		self._value = init_value
		self.mylock = threading.RLock()

	def __int__(self):
		return int(self._value)

	def __float__(self):
	    return float(self._value)

	def __eq__(self, cmp_value):
		try:
			self.mylock.acquire()
			return self._value == cmp_value
		finally:
			self.mylock.release()

	def __lt__(self, cmp_value):
		try:
			self.mylock.acquire()
			return self._value < cmp_value
		finally:
			self.mylock.release()

	def __le__(self, cmp_value):
		try:
			self.mylock.acquire()
			return self._value <= cmp_value
		finally:
			self.mylock.release()

	def __ne__(self, cmp_value):
		try:
			self.mylock.acquire()
			return self._value != cmp_value
		finally:
			self.mylock.release()

	def __gt__(self, cmp_value):
		try:
			self.mylock.acquire()
			return self._value > cmp_value
		finally:
			self.mylock.release()

	def __ge__(self, cmp_value):
		try:
			self.mylock.acquire()
			return self._value >= cmp_value
		finally:
			self.mylock.release()

	def __add__(self, add_value):
		try:
			self.mylock.acquire()
			return self._value + int(add_value)
		finally:
			self.mylock.release()

	def __iadd__(self, add_value):
		try:
			self.mylock.acquire()
			self._value += int(add_value)
			return self._value
		finally:
			self.mylock.release()

	def _add(self, value):
		try:
			self.mylock.acquire()
			return self._value
		finally:
			self._value += int(value)
			self.mylock.release()

	def __sub__(self, sub_value):
		try:
			self.mylock.acquire()
			return self._value - int(sub_value)
		finally:
			self.mylock.release()

	def __isub__(self, sub_value):
		try:
			self.mylock.acquire()
			self._value -= int(sub_value)
			return self._value
		finally:
			self.mylock.release()

	def _sub(self, value):
		try:
			self.mylock.acquire()
			return self._value
		finally:
			self._value -= int(value)
			self.mylock.release()

	def __mul__(self, multiple_value):
		try:
			self.mylock.acquire()
			return self._value * int(multiple_value)
		finally:
			self.mylock.release()

	def __imul__(self, multiple_value):
		try:
			self.mylock.acquire()
			self._value *= int(multiple_value)
			return self._value
		finally:
			self.mylock.release()

	def _mul(self, value):
		try:
			self.mylock.acquire()
			return self._value
		finally:
			self._value *= int(value)
			self.mylock.release()

	def __div__(self, div_value):
		try:
			self.mylock.acquire()
			return self._value / int(div_value)
		finally:
			self.mylock.release()

	def __idiv__(self, div_value):
		try:
			self.mylock.acquire()
			self._value /= int(div_value)
			return self._value
		finally:
			self.mylock.release()

	def _div(self, value):
		try:
			self.mylock.acquire()
			return self._value
		finally:
			self._value /= int(value)
			self.mylock.release()

	def __mod__(self, mod_value):
		try:
			self.mylock.acquire()
			return self._value % int(mod_value)
		finally:
			self.mylock.release()

	def __imod__(self, mod_value):
		try:
			self.mylock.acquire()
			self._value %= int(mod_value)
			return self._value
		finally:
			self.mylock.release()

	def _mod(self, value):
		try:
			self.mylock.acquire()
			return self._value
		finally:
			self._value %= int(value)
			self.mylock.release()

	def __pow__(self, pow_value):
		try:
			self.mylock.acquire()
			return self._value ** int(pow_value)
		finally:
			self.mylock.release()

	def __ipow__(self, pow_value):
		try:
			self.mylock.acquire()
			self._value **= int(pow_value)
			return self._value
		finally:
			self.mylock.release()

	def _pow(self, value):
		try:
			self.mylock.acquire()
			return self._value
		finally:
			self._value **= int(value)
			self.mylock.release()

	def __and__(self, and_value):
		try:
			self.mylock.acquire()
			return self._value & int(and_value)
		finally:
			self.mylock.release()

	def __iand__(self, and_value):
		try:
			self.mylock.acquire()
			self._value &= int(and_value)
			return self._value
		finally:
			self.mylock.release()

	def _and(self, value):
		try:
			self.mylock.acquire()
			return self._value
		finally:
			self._value &= int(value)
			self.mylock.release()

	def __or__(self, or_value):
		try:
			self.mylock.acquire()
			return self._value | int(or_value)
		finally:
			self.mylock.release()

	def __ior__(self, or_value):
		try:
			self.mylock.acquire()
			self._value |= int(or_value)
			return self._value
		finally:
			self.mylock.release()

	def _or(self, value):
		try:
			self.mylock.acquire()
			return self._value
		finally:
			self._value |= int(value)
			self.mylock.release()

	def __xor__(self, xor_value):
		try:
			self.mylock.acquire()
			return self._value ^ int(xor_value)
		finally:
			self.mylock.release()

	def __ixor__(self, xor_value):
		try:
			self.mylock.acquire()
			self._value ^= int(xor_value)
			return self._value
		finally:
			self.mylock.release()

	def _xor(self, value):
		try:
			self.mylock.acquire()
			return self._value
		finally:
			self._value ^= int(value)
			self.mylock.release()

	def __abs__(self):
		try:
			self.mylock.acquire()
			return abs(self._value)
		finally:
			self.mylock.release()


if __name__ == '__main__':
	i = AtomInt(5)
	l = AtomInt(-9)
	print i + l
	print int(i), int(l)
	print i._pow(7)
	print int(i)
	print i == l
