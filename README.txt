
This is a workaround that fixes a problem that occurs in gdb on some systems.  It replaces "system" and "popen" and replaces them with functions that achieve the same effect by giving the task to another program, which is communicated with via temporary files.

both "system" and "popen" call "fork".  Forking in a multithreaded program can confuse gdb on some systems, causing the program first to jump to the thread that's called fork, then when hitting next it will act as if you've used "continue" instead of going to the next instruction.  

Debugging with this limitation is extremely difficult.  I noticed this problem after updating from Ubuntu 18.04 to 20.04.  I made this primarily for myself, but considering how troublesome this bug is, I figure some may be very interested in this workaround.



platforms: Linux, probably most other *nix

build: 
	first "chmod +x ./build.sh" and "chmod +x ./clean.sh"
	./build.sh

install/use: 
	source gdbinit_fork_hack.py somewhere in a command defined in your to .gdbinit, for example:
		in ~/.gdbinit:
			#it's important that you source gdbinit_fork_hack.py with a real instead of symbolic path, meaning don't use "~" here, do something like /home/dylan/tools/GdbForkHack/gdbinit_fork_hack.py.
			so <full-real-path-to-GdbForkHack>/gdbinit_fork_hack.py
			so ~/.gdbinit.py
		in ~/.gdbinit.py:
			#ss for start, except different
			class ss(gdb.Command):
				def __init__ (self):
					super(ss, self).__init__ ("ss", gdb.COMMAND_USER)
				def invoke (self, arg, from_tty):
					fork_hack_init()
			ss()
	then use the "ss" command to  in gdb before debugging your program

