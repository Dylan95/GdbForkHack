
import os

gdbForkHackDir = os.path.dirname(os.path.realpath(__file__))

def fork_hack_init():
	gdb.execute("set exec-wrapper env 'LD_PRELOAD=" + os.path.join(gdbForkHackDir, "fork_hack.so") + "'")
	gdb.execute("start")
	gdb.execute("p ::fork_hack::fork_hack_init()")
	pidStr = gdbExecReturn("info inferior")
	if(pidStr[-1]=="\n"):
		pidStr=pidStr[:-1]
	pidStr = pidStr[pidStr.rfind("\n") :]
	tokens = pidStr.split();
	#input(tokens)
	pid = int(tokens[3])
	print("inferior PID: " + str(pid))
	os.system(os.path.join(gdbForkHackDir, "fork_hack_watcher.exe") + " " + str(pid) + " &")

