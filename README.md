# VerifyAgainstFlowChart

A program that can parse a C source file (with includes) and verifies it against a flowchart made with
[yEd](https://www.yworks.com/products/yed) or generate a flowchart from a C program.

This program is being developed for a particular use case for myself and very likely will not
work with an arbitrary C program and/or yED flowchart diagrams.

## Status

The following has been implemented:
- [`c_parser.c`](src/c_parser.c): parse a C program
- [`flowchart_parser.c`](src/flowchart_parser.c): parses the flowchart in a `graphml` file
- [`c_blocks.c`](src/c_blocks.c): convert parsed program into flowchart similar blocks
- [`compare.c`](src/compare.c): compare flowchart of parsed program with flowchart
- [`output_flowchart.c`](src/output_flowchart.c): outputs flowchart of parsed program

Call it with `verifyafc [-I <include folder>] [<c filename>|<graphml filename|-o <graphml filename>]*`.
System include files are read from the `include` folder relative to the executable.
So, place it in the root folder of the repository to the use the drop-in files `include`
folder of the repository.

The program prints warnings when parsing the C program.
The Program prints compare results between flowchart of program and the flowchart
after processing the input and no flowchart output has been generated.

## Build

To build, issue `gcc -Wall -Werror verifyafc.c -o ../verifyafc` in `src` folder.
Then run from that location (or copy the `include` folder to location from which it is
executed if the C source depends on system include files).

