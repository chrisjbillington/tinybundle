BOOTSTRAPPER_SIZE = 15000

import os
if os.path.exists('tinybundle'):
    os.system('rm tinybundle')
assert not os.system('gcc tinybundle.c -o tinybundle -DBOOTSTRAPPER_SIZE=%d'%BOOTSTRAPPER_SIZE)
assert not os.system('gcc bootstrapper.c -o bootstrapper -DBOOTSTRAPPER_SIZE=%d'%BOOTSTRAPPER_SIZE)
actual_bootstrapper_size = os.path.getsize('bootstrapper')
required_padding = BOOTSTRAPPER_SIZE - actual_bootstrapper_size
print '[build.py: bootstrapper size is %d bytes out of %d]'%(actual_bootstrapper_size,BOOTSTRAPPER_SIZE)
assert required_padding >= 0
with open('bootstrapper','ab') as f:
    f.write('\x00'*required_padding)

assert not os.system('cat bootstrapper >> tinybundle')
assert not os.system('rm bootstrapper')



# test:
print '[build.py: testing...]'
os.system('rm -f test/test test/test_bundled')
assert not os.system('gcc test/test.c -o test/test')
assert not os.system('./tinybundle test/test test/file_1.txt test/file_2.txt test/test_bundled')
assert not os.system('test/test_bundled')
