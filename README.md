# CPS-Project-Gecode

C++ implementation of the Gecode project for finding a NLS(Nor Logical Synthesis) circuit equivalent to the one received as a parameter.


## Important things before running

First take a look at `Makefile` in `src/` folder. It holds the configuration of Gecode, that is the installation directory of the framework to be used.
Make the needed changes to adapt it to your system (I'm assuming you have already installed Gecode in your computer). If you don't have Gecode yet, they explain [how to install](https://www.gecode.org/doc/2.2.0/reference/PageComp.html) the framework in their website.

## Execute the program

The steps to execute the program are the following:

- Move to `src/` folder and execute `make` command.

- To execute any instance run `./Nor < <instance-name>`