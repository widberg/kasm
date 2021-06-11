# kasm

A MIPS-like virtual machine, compiler, and assembler.

## Compilers Targeting KASM

* [Kiwi Language Compiler](https://github.com/litala123/kiwi-lang) by [litala123](https://github.com/litala123)

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

* CMake >= 3.2

### Checkout

```sh
git clone https://github.com/widberg/kasm.git
cd kasm
```

### Build

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

## KASM Environment

### Tools

* kasm - Assembler
  - `kasm asm source.kasm o.kexe`
* kdsm - Disassembler
  - `kasm dsm o.kexe source.kasm`
* kvm - Virtual Machine
  - `kasm vm o.kexe`
* kdbg - Debugger
* klang - K Structured Programming Language Compiler

## kasm/kvm

### Directives

| Directive | Description |
| --- | --- |
| .text | Move the cursor to the first unwritten byte of the text segment |
| .data | Move the cursor to the first unwritten byte of the data segment |
| .word i | a\[, i\|a\]... Align the cursor to the nearest word boundary and writes each immediate or memory direct address as a word to the data segment in sequence |
| .word i | a:X Align the cursor to the nearest word boundary and writes the immediate or labeled address as a word to the data segment the specified number of times |
| .byte i | a\[, i\|a\]... Write each immediate or labeled address as a byte to the data segment in sequence |
| .byte i | a:X Write the immediate or memory direct address as a byte to the data segment the specified number of times |
| .align X | Align the cursor to the nearest 2^X address |
| .ascii "string" | Write the characters of `string` to the data segment as bytes in sequence |
| .asciiz "string" | Write the characters of `string` to the data segment as bytes in sequence followed by a null byte |
| .space X | Move the cursor forward by X bytes |
| .include "file" | Insert the contents of `file` into the input stream in place of this directive. If a relative path is used, the current working directory is searched first, then if no file was found, the directory containing the assembler executable is searched. |
| .message "msg" | Print `msg` to the standard output stream |
| .error "msg" | Print `msg` to the standard output stream and throw an assembler exception |
| .dbg "msg" | Dummy directive to test assembler |
| .dbgbp | In debug builds cause a debug breakpoint in the assembler |
| .define IDENTIFIER XXXXX | Define an inline macro |
| .macro IDENTIFIER([parameter0[, parameter1]...]) | Begin a macro function definition |
| .end | End macro function definition |
| IDENTIFIER([argument0[, argument1]...]) | Insert the contents of the macro body with parameter replacement into the input stream in place of this directive |

### Registers

| ID | Name | Conventional Purpose |
| --- | --- | --- |
| 0 | zero | Hard-wired to 0 |
| 1 | at | Reserved for pseudo-instructions |
| 2-3 | v0-v1 | Return values from functions |
| 4-7 | a0-a3 | Arguments to functions |
| 8-17 | t0-t9 | Temporary data |
| 18-27 | s0-s9 | Saved registers |
| 28 | gp | Global Area Pointer |
| 29 | sp | Stack Pointer |
| 30 | fp | Frame Pointer |
| 31 | ra | Return Address |

### Address Encoding

Instructions are (1 word | 4 bytes | 32 bits) long.

Addresses are stored inline with the instruction. Bits used for the address fields may not be used for the instruction fields.

```text
|------DDDDDDDDDDDDDDDDDDDDDDDDDD| Direct Address Absolute
|----------------AAAAAAAAAAAAAAAA| Address Offset
|-----------RRRRR----------------| Register
```

### Addressing Modes

| Mode | Format | Effective Address |
| --- | --- | --- |
| Memory Direct | label | Address of label |
| Register Indirect | ($s) | Contents of `$s` |
| Immediate Offset | i($s) | Contents of `$s` + `i` |
| Label Offset | label($s) | Contents of `$s` + address of label |
| Label + Immediate | label+i | Address of label + `i` |
| Label + Immediate Offset | label+i($s) | Address of label + `i` + contents of `$s` |

### Instruction Encoding

Instructions are (1 word | 4 bytes | 32 bits) long.

```text
|OOOOOO--------------------------| Opcode
|------AAAAABBBBBCCCCC-----------| Register slots A, B, and C
|----------------IIIIIIIIIIIIIIII| Immediate
```

### Instruction Set

| KASM | Operation | Encoding |
| --- | --- | --- |
| add $d, $s, $t | $d = $s + $t; advancePC(); | 0000 00dd ddds ssss tttt t--- ---- ---- |
| addi $d, $s, i | $d = $s + i; advancePC(); | 0000 01dd ddds ssss iiii iiii iiii iiii |
| addiu $d, $s, i  | $d = $s + i; advancePc(); |  |
| addu $d, $s, $t  | $d = $s + $t; advancePc(); |  |
| and $d, $s, $t  | $d = $s & $t; advancePc(); |  |
| andi $d, $s, i  | $d = $s & i; advancePc(); |  |
| beq $f, $s, address | if ($f == $s) setPc(address); |  |
| bgez $f, address  | if ($f >= 0) setPc(address); |  |
| bgezal $f, address  | if ($f >= 0) { advancePc(); $ra = getPc(); setPc(address) }; |  |
| bgtz $f, address  | if ($f > 0) setPc(address); |  |
| blez $f, address  | if ($f <= 0) setPc(address); |  |
| bltz $f, address  | if ($f < 0) setPc(address); |  |
| bltzal $f, address  | if ($f < 0) { advancePc(); $ra = getPc(); setPc(address) }; |  |
| bne $d, $f, address | if ($f != 0) setPc(address); |  |
| div $f, $s  | lo = $f / $s; hi = $f % $s |  |
| divu $f, $s  | lo = $f / $s; hi = $f % $s |  |
| j address  | setPc(address); |  |
| jal address  | advancePc(); $ra = getPc(); setPc(address); |  |
| jalr $f, $s  | advancePc(); $s = getPc(); setPc($f); |  |
| jr $f | setPc($f); |  |
| lb $d, address  | $d = memory\[address\]; advancePc(); |  |
| lui $d, i  | $d = i << 16; advancePc(); |  |
| lw $d, address  | $d = \*(uint32_t*)&memory\[address\]; advancePc(); |  |
| mfhi $d  | $d = hi; advancePc(); |  |
| mflo $d  | $d = lo; advancePc(); |  |
| mult $f, $s  | lo = $f \* $s; hi = ($f \* $s) >> 32; advancePc(); |  |
| multu $f, $s  | lo = $f \* $s; hi = ($f \* $s) >> 32; advancePc(); |  |
| nor $d, $s, $t  | $d = ~($s \| $t); advancePc(); |  |
| or $d, $s, $t  | $d = $s \| $t; advancePc(); |  |
| ori $d, $s, i  | $d = $s \| i; advancePc(); |  |
| sb $f, address  | memory\[address\] = $f & 0xFF; advancePc(); |  |
| seq $d, $s, $t  | $d = $s == $t; advancePc(); |  |
| sll $d, $s, i  | $d = $s << i; advancePc(); |  |
| sllv $d, $s, $t  | $d = $s << $t; advancePc(); |  |
| slt $d, $s, $t  | $d = $s < $t; advancePc(); |  |
| slti $d, $s, i  | $d = $s < i; advancePc(); |  |
| sltiu $d, $s, i  | $d = $s < i; advancePc(); |  |
| sltu $d, $s, $t  | $d = $s < $t; advancePc(); |  |
| sne $d, $s, $t  | $d = $s != $t; advancePc(); |  |
| sra $d, $s, i  | $d = (signed)$s >> i; advancePc(); |  |
| srl $d, $s, i  | $d = $s >> i; advancePc(); |  |
| srlv $d, $s, $t  | $d = $s >> $t; advancePc(); |  |
| sub $d, $s, $t  | $d = $s - $t; advancePc(); |  |
| subu $d, $s, $t  | $d = $s - $t; advancePc(); |  |
| sw $d, address  | \*(uint32_t*)&memory\[address\] = $f; advancePc(); |  |
| sys | systemCall(); advancePc(); |  |
| xor $d, $s, $t  | $d = $s ^ $t; advancePc(); |  |
| xori $d, $s, i  | $d = $s ^ i; advancePc(); |  |

### Pseudoinstruction Set

| KASM | Operation |
| --- | --- |
| add $d, $s, i | addi $d, $s, i |
| b address | beq $zero, $zero, address |
| bal address | bgezal $zero, address |
| beq $f, i, address | ori $at, $zero, i; beq $f, $at, address |
| beqz $f, address | beq $f, $zero, address |
| bge $f, $s, address | slt $at, $f, $s; beq $at, $zero, address |
| bgt $f, $s, address | slt $at, $s, $f; bne $at, $zero, address |
| bgtu $f, $s, address | sltu $at, $f, $s; beq $at, $zero, address |
| ble $f, $s, address | slt $at, $s, $f; beq $at, $zero, address |
| blt $f, $s, address | slt $at, $f, $s; beq $at, $zero, address |
| bne $f, i, address | ori $at, $zero, i; bne $f, $at, address |
| call address | jal address |
| clr $d | or $d, $zero, $zero |
| copy $d, $s | or $d, $s, $zero |
| div $d, $s, $t | div $s, $t; mflo $d |
| enter | addi $sp, $sp, -4; sw $ra, 0($sp); addi $sp, $sp,-4; sw $fp, 0($sp); or $fp, $sp, $zero |
| jalr $f | jalr $f, $ra |
| la $d, address | lui $d, address >> 16; ori $d, $d, address & 0xFFFF |
| li $d, i | lui $d, i >> 16; ori $d, $d, i & 0xFFFF |
| mult $d, $s, $t | mult $s, $t; mflo $d |
| nop | sll $zero, $zero, 0 |
| not $d, $t | nor $d, $t, $zero |
| popb $f | lb $f, 0($sp); addi $sp, $sp, 1 |
| popw $f | lw $f, 0($sp); addi $sp, $sp, 4 |
| pushb $d | addi $sp, $sp, -1; sb $d, 0($sp)|
| pushw $d | addi $sp, $sp, -4; sw $d, 0($sp) |
| rem $d, $s, $t | div $s, $t; mfhi $d |
| ret | or $sp, $fp, $zero; lw $fp, 0($sp); addi $sp, $sp, 4; lw $ra, 0($sp); addi $sp, $sp, 4; jr $ra |

### System Calls

System services are called by storing the service's code in the `$a0` register and executing the `sys` instruction. Some services require additional arguments to be executed; these arguments are stored in the `$ax` registers according to the table below. Some services return values; these return values can be accessed by reading them from the `$vx` registers according to the table below.

| Code | Service | Arguments | Returns |
| --- | --- | --- | --- |
| 0 | exit | $a1 = exit code |  |
| 1 | read_int |  | $a0 = word read |
| 2 | write_int | $a0 = word to write |  |
| 3 | read_char |  | $a0 = char read |
| 4 | write_char | $a0 = char to write |  |
| 5 | read_string | $a0 = buffer address, $a1 = buffer size |  |
| 6 | write_string | $a0 = null terminated buffer address |  |
| 7 | allocate | $a0 = size | $v0 = heap address |
| 8 | deallocate | $a0 = heap address |  |
| 9 | open_file | $a0 = file name buffer address, $a1 = mode | $v0 = file handle |
| 10 | close_file | $a0 = file handle |  |
| 11 | seek | $a0 = file handle, $a1 = distance, $a2 = mode |  |

### Standard Macro Library

The KASM Standard Macro Library is located in `std.kasm`.

## klang

### Directives

Directives must appear at the beginning of a line, excluding whitespace characters and comments.

| Directive | Description |
| --- | --- |
| %include "file" | Insert the contents of `file` into the input stream in place of this directive. If a relative path is used, the current working directory is searched first, then if no file was found, the directory containing the assembler executable is searched. |

### Types

| Name | Description |
| --- | --- |
| u8 | 1 byte unsigned integer |
| u16 | 2 byte unsigned integer |
| u32 | 4 byte unsigned integer |
| s8 | 1 byte signed 2s compliment integer |
| s16 | 2 byte signed 2s compliment integer |
| s32 | 4 byte signed 2s compliment integer |
| f32 | 4 byte IEEE-754 floating point number |
| f64 | 8 byte IEEE-754 floating point number |

Pointer types are denoted by an asterisk, `*`, following a type name or `void`.

### void

When used as a function return type, the void keyword specifies that the function does not return a value. When used in the declaration of a pointer, void specifies that the pointer is "universal."

### Type Specifiers

| Name | Description |
| --- | --- |
| const | The variable will not change after initialization |
| static | The variable will be stored in the data segment and retain its value across function calls after initialization |

### Expression Specifiers

| Name | Description |
| --- | --- |
| comptime | The expression's value will be evaluated at compile time |
| inline | Replaces function calls in the expression with the function definition |
| sizeof | Evaluates to size of the expression's type at compile time |

### Function Specifiers

| Name | Description |
| --- | --- |
| inline | Replaces calls to the function with the function definition |

### Inline KASM

The keyword `kasm` followed by an `{` will instruct the compiler to evaluate the proceeding text as if it were kasm. This will continue until the compiler encounters a `}` outside of a comment, string literal, or character literal.

### Intrinsics

| Name | Description |
| --- | --- |
| nullptr | Evaluates to a void pointer with the value 0 |
| true | Evaluates to a u8 with the value 1 |
| false | Evaluates to a u8 with the value 0 |

### Operators

### Control Structures

#### if, if/else

#### switch

#### while, do/while

#### for

#### return

#### Labeled Statement

#### goto

### Data Structures

#### enum

#### struct

#### union

### namespace

