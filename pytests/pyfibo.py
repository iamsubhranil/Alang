# Python program to display the Fibonacci sequence up to n-th term using recursive functions
import time
def recur_fibo(n):
   """Recursive function to
   print Fibonacci sequence"""
   if n <= 1:
       return n
   else:
       return(recur_fibo(n-1) + recur_fibo(n-2))

# Change this value for a different result
nterms = 28

# uncomment to take input from the user
#nterms = int(input("How many terms? "))

# check if the number of terms is valid
if nterms <= 0:
   print("Plese enter a positive integer")
else:
   print("Fibonacci sequence : ")
   t0 = time.clock()
   i = 0
   while i < 5:
       print(recur_fibo(28))
       i = i + 1
   t0 = time.clock() - t0
   print("Took ", t0)
