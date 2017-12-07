import time
t0 = time.clock()
i = 0
while(i < 100000000):
    i = i + 1
t0 = time.clock() - t0
print("Took ",t0,"s")
