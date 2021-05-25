# Steps to run the Simulator

1. Clone this repository and run the `make all` command.  
This will produce the executable named "simulator.exe"
2. Run the command `./simulator.exe <num_cores> <num_cycles> <N file names> <row_access_delay> <col_access_delay>`
    * <num_cores> = Number of cores you are running on. (If you want to run on more than 16 cores, then you need to change the MAX_CORES value in core.hpp file
    * <num_cycles> = Number of cycles for which you want to run the Simulator for. After that the stops the execution of all the units.
    * \<N file names> = Since we are running on N cores, so we need to pass the 'N' MIPS programs, each of them will run on a separate core. PS. A core must not use a DRAM row used by another row, in that case an exception is thrown.
    * <row_access_delay> = Row Access delay in cycles of DRAM
    * <col_access_delay> = Column Access Delay in cycles of DRAM
3. This will print the what happens in the Processors, MRM, and DRAM in every clock cyle on the console. In the end the contents of Registers and DRAM memory are also printed.

# Basic Description of Folders

Every folder contains a Design Document and contains information on how to run the code in the I/O section of the document. Some basic information about them is described below.

### QtSpim setup

This contains the information for installing QtSpim on your machine.  
Some basic MIPS code are also included for test run.

### Area under curve MIPS code

MIPS code Finding the algebraic area under curve of Convex polygon.  
Used basic MIPS operations like reading integer input, addition, subtraction, etc.

### Evaluating Post-Fix Expr MIPS code

MIPS code to evaluate a Post-fix expression.  
Reads a string input and uses "$sp" register as the stack.

### MIPS ISA Interpreter

This takes input a MIPS code and executes the instructions.  
Prints appropriate error in case of some fault in the input code given.

### MIPS ISA with DRAM

To handle the memory, we will us the DRAM from now on. Since accessing rows and colums of DRAM require more than a clock cycle, we have incorporated that delay in the simulator.  
Task_1 is the simulator that has blocking memory. That is it waits for the "lw" and "sw" instruction to finish.  
Task_2 is the simulator with non-blocking memory. It continues to execute the further instructions as long as they are safe to run.

### DRAM Requests Reordering

In the Non-blocking case we see that it continues to execute the instructions untill any unsafe instruction.  
In this we will introduce a wait buffer of fixed size, it stores all the DRAM requests, and sends them to DRAM after Re-ordering the instructions.  
For re-ordering it will cluser the instructions with same row access and will keep executing it unless their is some unsafe instruction or row gets empty.  
Other optimizations like Forwarding, removing redundant lw instructions was also done.

### Multicore MIPS ISA with MRM and DRAM Simulator

As mentioned earlier we are performing Re-ordering of instructions. In hardware this is done using a Memory Request Manager (MRM).  
We have designed the hardware of this MRM and incorporated the MRM in the Simulator. Now it includes the clock by clock activity of MRM as well.  
We have also extended the simulator to a multi-core scenario. It takes 'N' MIPS ISA as input and processes all of them. If there is some error in a file then the execution of that core stops.