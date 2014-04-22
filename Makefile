# TODO: bug is exposed with -O3
cc=gcc -O2

default: tinybundle

tinybundle: bootstrapper
	# Compile tinybundle, which is itself bundled with bootstrapper
	$(cc) src/tinybundle.c -o bin/tinybundle
	cat bin/bootstrapper >> bin/tinybundle
	rm bin/bootstrapper

bootstrapper:
	# Create dummy file so that we can compile
	echo "// Generated file do not edit\n#define BOOTSTRAPPER_SIZE 0" > src/bootstrapper.h
	mkdir -p bin
	# Compile the first time and record file size in the header
	$(cc) src/bootstrapper.c -o bin/bootstrapper
	size1=`wc -c < bin/bootstrapper`; \
		echo "// Generated file do not edit\n#define BOOTSTRAPPER_SIZE $$size1" > src/bootstrapper.h; \
		echo "bootstrapper size in first compile: $$size1";
	# Compile a second time, so that the binary knows its own size
	$(cc) src/bootstrapper.c -o bin/bootstrapper
	size2=`wc -c < bin/bootstrapper`; \
		echo "bootstrapper size in second compile: $$size2";
	
test: default
	# Make and run test program
	cd src/test && make bundled
	mv src/test/test_bundled bin/
	bin/test_bundled

clean:
	rm -f bin/bootstrapper bin/tinybundle bin/test_bundled
	cd src/test && make clean
	rm src/bootstrapper.h

