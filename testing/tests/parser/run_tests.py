"""
This runs the binary
"""

import os
import subprocess

binary = compiler
input_dir = "../../../snippets"
dir = os.listdir(input_dir)

print(input_dir + 'content:')
print(dir)

for file_name in dir:
    args = ("../../../" + binary, (input_dir + '/' + file_name))

    popen = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    popen.wait()
    output = popen.stdout.read()
    returnCode = popen.poll()
    print(output + '\nr:', returnCode)

print('---finished---')
