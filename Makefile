cc=gcc -O3 -Wno-unused-result

UNAME := $(shell uname)
ifeq ($(UNAME), Windows_NT)
extension=.exe
endif

default: tinybundle
    
tinybundle: bootstrapper
	# Compile tinybundle, which is itself bundled with bootstrapper
	$(cc) src/tinybundle.c -o bin/tinybundle$(extension)
	cat bin/bootstrapper$(extension) >> bin/tinybundle$(extension)
	rm bin/bootstrapper$(extension)

bootstrapper:
	# Create dummy file so that we can compile
	echo "// Generated file do not edit" > src/bootstrapper.h
	echo "#define BOOTSTRAPPER_SIZE 0" >> src/bootstrapper.h
	mkdir -p bin
	# Compile the first time and record file size in the header
	$(cc) src/bootstrapper.c -o bin/bootstrapper$(extension)
	size1=`wc -c < bin/bootstrapper$(extension)` && \
		echo "// Generated file do not edit" > src/bootstrapper.h && \
		echo "#define BOOTSTRAPPER_SIZE $$size1" >> src/bootstrapper.h && \
		echo "bootstrapper size in first compile: $$size1"
	# Compile a second time, so that the binary knows its own size
	$(cc) src/bootstrapper.c -o bin/bootstrapper$(extension)
	size2=`wc -c < bin/bootstrapper$(extension)` && \
		echo "bootstrapper size in second compile: $$size2";
	
test: default
	# Make and run test program
	cd src/test && make bundled
	mv src/test/test_bundled$(extension) bin/
	bin/test_bundled$(extension)

clean:
	rm -f bin/bootstrapper$(extension) bin/tinybundle$(extension) bin/test_bundled$(extension)
	cd src/test && make clean
	rm src/bootstrapper.h

