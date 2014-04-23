cc=gcc

UNAME := $(shell uname)
ifeq ($(UNAME), Windows_NT)
extension=.exe
endif

tinybundle=./../../bin/tinybundle$(extension)

default:
	$(cc) test.c -o test$(extension)
	
bundled: default
	$(tinybundle) test$(extension) file_1.txt file_2.txt test_bundled$(extension)
	rm test$(extension)
	
clean:
	rm -f test$(extension) test_bundled$(extension)
	
