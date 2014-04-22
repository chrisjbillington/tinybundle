BOOTSTRAPPER_SIZE = 15000

import os

os.system('rm bundler')
assert not os.system('gcc bundler.c -o bundler -DBOOTSTRAPPER_SIZE=%s'%BOOTSTRAPPER_SIZE)
assert not os.system('gcc bootstrapper.c -o bootstrapper -DBOOTSTRAPPER_SIZE=%s'%BOOTSTRAPPER_SIZE)
actual_bootstrapper_size = os.path.getsize('bootstrapper')
required_padding = BOOTSTRAPPER_SIZE - actual_bootstrapper_size
print 'build.py: bootstrapper size is %d bytes out of %s'%(actual_bootstrapper_size,BOOTSTRAPPER_SIZE)
assert required_padding >= 0
with open('bootstrapper','ab') as f:
    f.write('\x00'*required_padding)

assert not os.system('cat bootstrapper >> bundler')
assert not os.system('rm bootstrapper')



# test:
os.system('rm test/out test/hello')
assert not os.system('gcc test/hello.c -o test/hello')
assert not os.system('./bundler test/hello test/file_1.txt test/file_2.txt test/out')
assert not os.system('test/out')
