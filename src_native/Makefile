all: Main.class shmem.dll mem-client.exe boxwrap.exe boxwrap2.exe target.exe boxlib.dll

Main.class: Main.java shmem.java
	javac Main.java shmem.java

shmem.class: shmem.java
	javac shmem.java

shmem.h: shmem.class
	javah shmem

shmem.dll: shmem.c shmem.h mem.h
	"C:\Program Files\CodeBlocks\MinGW\bin\gcc.exe" -Wall -Wl,--kill-at -Wl,--add-stdcall-alias -I"C:\\Program Files\\Java\\jdk1.8.0_271\\include" -I"C:\\Program Files\\Java\\jdk1.8.0_271\\include\\win32" -shared -o shmem.dll shmem.c

# Don't ask, ok? Not in the mood...
boxwrap.exe: boxwrap.c
	cd 32 && "C:\SysGCC\mingw32\bin\gcc.exe" -Wall -o ..\boxwrap.exe ..\boxwrap.c

boxwrap2.exe: boxwrap2.c
	cd 32 && "C:\SysGCC\mingw32\bin\gcc.exe" -Wall -o ..\boxwrap2.exe ..\boxwrap2.c

target.exe: target.c
	cd 32 && "C:\SysGCC\mingw32\bin\gcc.exe" -Wall -o ..\target.exe ..\target.c

boxlib.dll: boxlib.c mem.h
	cd 32 && "C:\SysGCC\mingw32\bin\gcc.exe" -Wall -shared -o ..\boxlib.dll ..\boxlib.c

mem-client.exe: mem-client.c mem.h
	"C:\Program Files\CodeBlocks\MinGW\bin\gcc.exe" -Wall -o mem-client.exe mem-client.c

test:
	java Main
