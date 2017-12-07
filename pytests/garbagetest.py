import time

class Test():
    value = 0
    def __init__(self, x):
        value = x

t0 = time.clock()
i = 0
while(i < 10000000):
    t = Test(i)
    i = i + 1

t0 = time.clock() - t0
print("Took ", t0, "s")
