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

✅ kasm - Assembler
✅ kdsm - Disassembler
✅ kvm - Virtual Machine
🟩 kdbg - Debugger
⬜ klang - K Structured Programming Language Compiler

### Instruction Encoding

Instructions are (1 word | 4 bytes | 32 bits) long.

```text
|OOOOOO--------------------------| Opcode
|------AAAAABBBBBCCCCC-----------| Register slots A, B, and C
|------DDDDDDDDDDDDDDDDDDDDDDDDDD| Direct address absolute
|----------------IIIIIIIIIIIIIIII| Immediate
|----------------FFFFFFFFFFFFFFFF| Direct address offset
```

### Instruction Set

| KASM | Operation | Encoding |
| --- | --- | --- |
| add $d, $s, $t | $d = $s + $t; advancePC(); | 0000 00dd ddds ssss tttt t--- ---- ---- |
| addi $d, $s, i | $d = $s + i; advancePC(); | 0000 01dd ddds ssss iiii iiii iiii iiii |

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

### System Calls

System services are called by storing the service's code in the `$a0` register and executing the `sys` instruction. Some services require additional arguments to be executed; these arguments are stored in the `$ax` registers according to the table below. Some services return values; these return values can be accessed by storing them in the `$vx` registers according to the table below.

| Code | Service | Arguments | Returns |
| --- | --- | --- | --- |
| 0 | exit | $a1 = exit code |  |
