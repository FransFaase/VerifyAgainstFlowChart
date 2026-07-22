# VerifyAgainstFlowChart

A program that can parse a C source file (with includes) and verify it against a flow-chart made with yEd.

This program is being developed for a particular commercial use case and might not be generally applicable.

## Status

A first version of the program to parse the input in `c_parser.c` has been implemented.
Call it with `c_parser [-I <include folder>] <c filename>`. System include files are read
from the `include` folder relative to the executable. So, place it in the root folder of
the repository to the use the drop-in files `include` folder of the repository.
