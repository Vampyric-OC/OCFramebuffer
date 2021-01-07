all: Main.class shmem.dll mem-client.exe

Main.class: Main.java shmem.java
	javac Main.java shmem.java

shmem.class: shmem.java
	javac shmem.java

shmem.h: shmem.class
	javah shmem
	touch shmem.h

shmem.dll: shmem.c shmem.h
	/c/Program\ Files/CodeBlocks/MinGW/bin/gcc -Wall -Wl,--kill-at -Wl,--add-stdcall-alias -I/c/Program\ Files/Java/jdk1.8.0_271/include -I/c/Program\ Files/Java/jdk1.8.0_271/include/win32 -shared -o shmem.dll shmem.c

mem-client.exe: mem-client.c
	/c/Program\ Files/CodeBlocks/MinGW/bin/gcc -Wall -o mem-client.exe mem-client.c

test:
	java Main
