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

| Name | Operation | Encoding |
| --- | --- | --- |
| Add | $d = $s + $t; advancePc(); | 0000 00dd ddds ssss tttt t--- ---- ---- |

### Registers

| ID | Name | Conventional Purpose |
| --- | --- | --- |
| 0 | zero | Hard-wired to 0 |
| 1 | at | Reserved for pseudo-instructions<sup>‡</sup> |
| 2-3 | v0-v1 | Return values from functions<sup>†</sup> |
| 4-7 | a0-a3 | Arguments to functions<sup>†</sup> |
| 8-15 | t0-t7 | Temporary data<sup>†</sup> |
| 16-23 | s0-s7 | Saved registers |
| 24-25 | t8-t9 | More temporary registers<sup>†</sup> |
| 26-27 | k0-k1 | Reserved for kernel<sup>‡</sup> |
| 28 | gp | Global Area Pointer |
| 29 | sp | Stack Pointer |
| 30 | fp | Frame Pointer |
| 31 | ra | Return Address<sup>†</sup> |

†<sub>Not preserved by subprograms.</sub>

‡<sub>Do not use.</sub>
