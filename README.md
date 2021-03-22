# Assembler-for-12-bit-processor
Final project in System Programming Lab course (Computer Science, The open University of Israel)

Program Objective:
 *****************
 This is an assembler program for an imaginary machine architecture.
 It processes files that their path is given as program argument one by one.
 Each file is checked for error and them translated to 1-3 output files - for file name file.as:
 	 1) file.obj - Object file that contains the machine code for instructions and initialized data segment.
 	 2) (optional) file.ent - file that holds a list of global labels and their addresses in the machine code.
 	 	 this file will not be generated if the code does not contain .entry directives.
	 3) (optional) file.ext - holds the addresses of words that should be later replaced with an external
	 	 	 	   label address
 Method of Operation:
 *******************
 The program reads each file line by line, each line is checked and normalized in terms of whitespace repetition
 and trailing/preceding whitespace truncation.
 Then, the lines are separated to instructions and directives while keeping track of all symbols witnessed in
 the source code in a symbol table, in order to detect use the of labels that haven't been assigned an address yet
 because data segment is not yet built at that point. entry definition are stored in ent_line array.
 After that, the code is built by mapping instructions ,values, opcodes, addressing methods from a structure
 called isa_struct (ISA for - Instruction Set Architecture), and address from the symbol table to a string
 containing also and the A, R, E flag, this string is saved in the global code_lines 2d array. Code lines that depend
 on addresses of labels who's addresses still unknown, are temporarily stored in code_line array as the label name only,
 to later be replaced when the data is built and given an address. if a label is declared extern a line is writen to ext_lines.
 In the last stage of processing, the directive part of line is scanned, for each directive that initializes data,
 the data is read from line and encoded to the data_lines 2d array, and the label (if exists) set to it
 is saved in the symbol table, the address of the first element of data is set to all temporary labels in code_lines.
 after the entire table is built, a scan for entry directives is performed to make sure no violations, building the .ent file.
 If all stages are done without errors, output files are generated.
 Structure:
 *********
 assembler.c - main function file, path processing functions.
 code_proc.c - contains functions to handle code processing.
 string_structs.c - contains data structures to help process text.
  	  	  	  	  	  symbol table structure, isa structure and line list structure, and their functions.
 Each has a .h file that is associated with it.
 Additional:
 **********
 The assembler will not stop processing a file (at least per stage) when an error detected, but no
 output products will be produce.
 This program was written with emphasis on simplicity, dynamic allocation was almost completely avoided.
 (used only in the symbol table in order to use with map function (which handles arrays of pointers).
 the only globals are the 2d arrays which store the strings to eventually printed to files, they were defined
 globals for the ease of sharing them between code processing functions and output writing function.
