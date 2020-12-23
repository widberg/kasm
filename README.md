<img align="left" src="https://raw.githubusercontent.com/widberg/kasm/logo.svg" width="120px"/>

[![Build status][actions-svg]][actions]

[actions-svg]:       https://github.com/widberg/kasm/workflows/build/badge.svg

[actions]:           https://github.com/widberg/kasm/actions?query=workflow%3Abuild

# kasm

A MIPS-like virtual machine, compiler, and assembler.

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
* kdsm - Disassembler
* kvm - Virtual Machine
* kdbg - Debugger
* klang - K Structured Programming Language Compiler

## kasm/kvm

### Directives

| Directive | Description |
| --- | --- |
| .text | Move the cursor to the first unwritten byte of the text segment |
| .data | Move the cursor to the first unwritten byte of the data segment |
| .word i|a[, i|a]... | Align the cursor to the nearest word boundary and writes each immediate or memory direct address as a word to the data segment in sequence |
| .word i|a:X | Align the cursor to the nearest word boundary and writes the immediate or labeled address as a word to the data segment the specified number of times |
| .byte i|a[, i|a]... | Write each immediate or labeled address as a byte to the data segment in sequence |
| .byte i|a:X | Write the immediate or memory direct address as a byte to the data segment the specified number of times |
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

Addresses are stored inline with the instruction. Bits used for the address fieldS may not be used for the instruction fields.

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

### System Calls

System services are called by storing the service's code in the `$a0` register and executing the `sys` instruction. Some services require additional arguments to be executed; these arguments are stored in the `$ax` registers according to the table below. Some services return values; these return values can be accessed by reading them from the `$vx` registers according to the table below.

| Code | Service | Arguments | Returns |
| --- | --- | --- | --- |
| 0 | exit | $a1 = exit code |  |

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

