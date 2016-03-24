import math

count = 0

a = 1125 * math.log((1 + 300 / 700), math.e)
b = 1125 * math.log((1 + 8000 / 700), math.e)
c = (b - a) / 25

for i in range(26):
	t = a + c * i
	b = 700 * (math.exp(t / 1125) - 1)
	b = math.floor(257 * b / 8000)
	count += 1
	#b = round(b, 2)
	print(b)

print()
print(count)
