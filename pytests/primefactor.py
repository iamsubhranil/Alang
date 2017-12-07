import time

def isPrime(n):
    i = 2
    while i<=n/2:
        if(n % i == 0):
            return False
        i = i + 1
    return True

num = int(input("Enter the number : "))
t0 = time.clock()
j = num/2
while j >= 2:
    if(num % j == 0):
        if(isPrime(j)):
            print(j, " is a prime factor of ", num)

    j = j - 1
t0 = time.clock() - t0
print("Took ",t0, "s")
