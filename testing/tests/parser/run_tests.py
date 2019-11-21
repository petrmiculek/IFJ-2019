"""
This runs the binary with all files in input_dir (snippets),
shows its output
and return code (r:_)
"""

import os
import subprocess

main_dir = "../../../"
binary = "compiler"
input_dir = "snippets"
dir = os.listdir(main_dir + input_dir)

for file_name in dir:
    current_file = (main_dir + input_dir + '/' + file_name)
    print(file_name)
    args = (main_dir + binary, current_file)

    popen = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    popen.wait()
    output = popen.stdout.read()
    returnCode = popen.poll()
    print(output + 'r:', returnCode, '\n---\n')
