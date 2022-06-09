# DCL

DeComp - a simulator of an old-school computer (all rights belong to Lviv Polytechinc National University). It's interesting to play with it until you have a laboratory excersise that, as teacher says, will take up to 200 of instructions. And, well, it's not really possible to hold in mind that big algorithm. So, that's why I made this language and wrote a comiller for it. 

## Syntax basics

I have no idea where should I mention it, but the compiller is case-insensitive, so, ```AND``` == ```and```, ```VariableName``` == ```variablename```...

Basically, the DCL (DeCompLanguage) code consits of two segments:
- Data segment
- Main segemnt

### Data segement

is where all the global variables are defined. The syntax is pretty simple:
```
.DATA
	variable1 10
	variable2 0o12
	variable3 0b1010
	variable4 0xA
```
The segment starts with ```.DATA``` (remebrer, case is ignored, so, it may be ```.data``` or even ```.dAtA```😐).  
Variable definition syntax is straightforward as well: ```<variable name> <its initial value>```. Initial values can be written in :
- deciamal (no prefix) 
- binary (```0b``` prefix)
- octal (```0o``` prefix)
- hexademical (```0x``` prefix)

There may be as many data segements as you want (0 - ∞)

### Main segment
is where you write all the instructions.
```
.MAIN
	ADD variable1 variable2
	NOT acm
	STOP
```
The segment starts with ```.MAIN``` (remember, case is ignored, so, it may be ```.main``` or even ```.mAiN```😐).  
The instructions syntax is straightforward as well: ```<instruction> <0 to 2 arguments>```.


## Instructions

there are 2 types of instructions: general and special ones.

### General instructions

| name    | arguments number |
|:-------:|:----------------:|
| add     | 2                |
| sub     | 2                |
| add     | 2                |
| or      | 2                |
| xor     | 2                |
| not     | 1                |
| output  | 1                |
| lsl     | 1                |
| lsr     | 1                |
| asl     | 1                |
| asr     | 1                |
| rol     | 1                |
| ror     | 1                |
| rcl     | 1                |
| rcr     | 1                |
| stop    | 0                |

These instructions will be just converted into 1 or 2 DeComp instructions. For example, 

- if we want to check the result of ```AND var1 var2```, DeComp will need to load the first variable and the run  ```AND``` with the second one (2 instructions);
- if we want to check the result of ```AND acm var2``` (acm means accumulator), DeComp will need to to just run ```AND``` with var2 (1 instruction).

### Special instructions

Special instructinos are instructions whose behaviour is slightly different from the general ones.
Here is a list of them with explanations:
- ```INPUT``` takes 1 argument - a variable name (then it will store read data in the variable) or "acm";
- ```LABEL``` takes 1 argument - a new label name that you can ```goto```;
- ```GOGO``` takes 1 argument - a label name to jump to
- ```mov``` takes 2 arguments and the value of the second argument will be copied to the first one;
- ```if``` statement should be written as ```if <condition> do <block of instructions> else <block of instructions> end``` (more on conditions syntax is in the [conditions](#conditions) section). ```else``` can be ommited;
- ```while``` statement should be written as ```while <condition> do <block of instructions> end```- (more on conditions in the [conditions](#conditions) seciton).

## Conditions

are used in the ```if``` and ```while``` statements.  
I cannot say if they are expressions or statements... It's a different thing.

### The syntax of unit

is pretty simmple: ```[ <expression> ] <sign condition>```.  
The unit consists of 2 parts:
- under-condition expression (it's optional)
- the actual condition

In the ```under-condition``` part you can write as many instructions as you want. It just holds a block of instructions result of which will be then "conditioned"

The ```actual condition``` part is just an operator. Here is a list of available values and its meanings:
| name | meaning           |
|:----:|:-----------------:|
| z    | "zero" sign is 1  |
| nz   | "sero" sign is 0  |
| s    | "sign" sign is 1  |
| ns   | "sign" sign is 0  |
| c    | "carry" sign is 1 |
| nc   | "carry" sign is 0 |

### The full condition syntax

consists of a list of condition units separated by ```and``` or ```or``` operators. Unfortunatelly, there is no custom grouping provided, so, condition units are separated into "or" groups by "and"s. So, condition like ```z or s and nc or ns or s``` will be grouped as ```(z or s) and (nc or ns or s)```

## Additional fature that I don't know where to put

### Pointers

As an argument to an instruction you can use pointers. Every variable can be considred as a pointer simply by adding a "&" before its name when passing to an instruction ❗❗❗ NOT IN THE DATA SECTION ❗❗❗ like this: ```AND acm &my_favourite_variable```.  
In this case will run ```AND``` instruction not for value that is stored in my favourite variable, but for value that is stored at the address that is equal to the value of my favourite variable. And, if it hold, for instance, 0xFF, then we'll look at the 0xFF address and take the value from it.

Be careful! Pointers can corrupt your code!

## Example code

This code will fill DeComp's memory from starting point of your choice (it inputs it) as many times as you say with the second parameter.
```
.DATA
	counter 0
	pointer 0
	n 0

.MAIN
	# inputing initial values
	input n
	input pointer

	# filling decomp memory with trash values
	while [ sub n counter ] nz and ns do
		mov &pointer pointer

		add pointer 1
		mov pointer acm

		add counter 1
		mov counter acm
	end
```

## Usage

To use the compiller, you'll need to run the compiler's .exe file with 2 arguments: <source file> <target file>. If there is no error, the compiller will show you DeComp instructions formatted for readability
