# VerifyAgainstFlowChart

A program that can parse a C source file (with includes) and verifies it against a flow-chart made with yEd.

This program is being developed for a particular commercial use case and might not be generally applicable.

## Status

The following has been implemented"
- `c_parser.c`: parse a C program
- `flowchart_parser.c`: parses the flow-chart in a `graphml` file
- `c_blocks.c`: convert parsed program into flow-chart similar blocks
- `compare.c`: compare flow-chart of parsed program with flow-chart
- `output_flowchart.c`: outputs flow-chart of parsed program

Call it with `verifyafc [-I <include folder>] [<c filename>|<graphml filename| -o <graphml filename>]*`.
System include files are read from the `include` folder relative to the executable.
So, place it in the root folder of the repository to the use the drop-in files `include`
folder of the repository.

The program prints warnings when parsing the C program.
The Program prints compare results between flow-chart of program and the flow-chart.

## Build

To build, issue `gcc -Wall -Werror verifyafc.c -o ../verifyafc` in `src` folder.
Then run from that location (or copy the `include` folder to location from which it is
executed if the C source depends on system include files).

