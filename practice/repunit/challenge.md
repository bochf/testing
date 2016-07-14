# Repunit

A number consisting entirely of ones is called a repunit. We shall define R(k) to be a repunit of length k; for example, R(6)=111111.

Given that n is a positive integer and gcd(n,10)=1, it can be shown that there always exists a value, k, for which R(k) is divisible by n, and let A(n) be the least such value of k; for example, A(7)=6 and A(41)=5.

The least value of n for which A(n) first exceeds ten is 17.

## Given n, compute A(n).

## Input Format

The first line of input contains T, the number of test cases.
Each test case consists of a single line containing single integer, n.

### Constraints
	gcd(n,10)=1

### Test files #1-2: 
	1≤T≤20000 
	1≤n≤106

### Test files #3-6: 
	1≤T≤100 
	1≤n≤1013

## Output Format

For each test case, output a single line containing a single integer, A(n).

## Sample Input
	2
	7
	41

## Sample Output
	6
	5

## Explanation
As mentioned in the problem statement, A(7)=6 and A(41)=5.
