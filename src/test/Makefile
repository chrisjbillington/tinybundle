cc=gcc
tinybundle=./../../bin/tinybundle

default:
	$(cc) test.c -o test
	
bundled: default
	$(tinybundle) test file_1.txt file_2.txt test_bundled
	rm test
	
clean:
	rm -f test test_bundled
