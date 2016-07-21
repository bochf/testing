# Mug Color
*Memory Limit: 1024MB Runtime Limit: 5s Score: 100*  

Jay S. has got himself in trouble! He had borrowed a friend's coffee mug and somehow lost it. As his friend will be extremely angry when he finds out about it, Jay has decided to buy his friend a replacement mug to try to control the damage. Unfortunately, Jay does not remember the color of the mug he had borrowed. He only knows that the color was one of **White, Black, Blue, Red or Yellow**.  
Jay goes around his office asking his colleagues if they are able to recall the color but his friends don't seem to remember the color of the mug either. What they do know is **what color the mug definitely was not.**  
Based on this information, help Jay figure out what the color of the mug was.  

## Input Specifications

Your program will take
An input N (1 ≤ N ≤ 1,000,000) which denotes the number of people Jay questions regarding the mug.
This will be followed by N strings S[1],S[2]...S[N] where S[I] denotes the response of person I to Jay's question which is what color the mug definitely was not. S[I] will also be only one of the 5 colors namely White, Black, Blue, Red or Yellow.

## Output Specifications

Based on the input, print out the color of the mug. The color of the mug can only be one of the 5 colors namely White, Black, Blue, Red or Yellow. 
You can safely assume that there always exists only one unique color that the mug can have.

## Sample Input/Output

```
INPUT
4
White
Yellow
Blue
Black
```
``` 
OUTPUT
Red
```
```
EXPLANATION
Jay's colleagues have mentioned every color except Red so the mug is Red in color
```
```
INPUT
9
White
Yellow
Blue
Black
Black
White
Yellow
Blue
Black
```
```
OUTPUT
Red
```
```
EXPLANATION
Similar to the above case, the only color not mentioned is Red
```