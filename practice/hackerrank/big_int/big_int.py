import sys

def sqr(x):
    return x * x

def main(n):
    x = 1
    for i in range(1, n):
        y = sqr(x) + 1
        print("sqr(%d) + 1 = %d" % (x, y))
        x = y

if __name__ == '__main__':
    n = int(sys.argv[1])
    main(n)
