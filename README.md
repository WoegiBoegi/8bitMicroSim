# an 8-bit microcomputer simulator
My best Project yet

this program simulates an 8 bit microcomputer (down to the CPU registers), I also included some handy scripts for compilation.

(I wrote this with ncurses and other gcc stuff, so I don't expect it to compile outside of unix systems, though I haven't tested it)

----------------
USAGE:

install the dependencies (just ncurses I think, see linking commands for external libraries used)

execute the compilation scripts or run the command yourself (don't forget to chmod +x them)

write a program in assembler (Instruction table included!) or use one of the provided demo files

assemble! ./assembler <source.asm> <destination.bin> (defaults to out.bin if no second argument in provided)

run! ./microsim <source.bin> 

------------------

I have designed the UI to be as intuitive as possible, with hints, keybinds and such always being displayed. You should be able to run stuff without any issues!
