BOOTSTRAPPER_SIZE = 15000

import os

os.system('rm bundler out')
assert not os.system('gcc bundler.c -o bundler')
assert not os.system('gcc bootstrapper.c -o bootstrapper')
actual_bootstrapper_size = os.path.getsize('bootstrapper')
required_padding = BOOTSTRAPPER_SIZE - actual_bootstrapper_size
assert required_padding >= 0
with open('bootstrapper','ab') as f:
    f.write('\x00'*required_padding)

assert not os.system('cat bootstrapper >> bundler')
assert not os.system('rm bootstrapper')

assert not os.system('./bundler file_1.txt file_2.txt out')
assert not os.system('./out')
