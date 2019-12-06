"""
This runs the binary with all files in input_dir (snippets),
shows its output
and return code (r:_)
"""

import os
import subprocess
import string

current_dir = os.path.split(os.getcwd())[1]

if current_dir == 'IFJ-2019':
    main_dir = "testing/tests/parser/"
else:
    # current_dir is most likely IFJ-2019/testing/tests/parser
    main_dir = "/"

# binary = "test_bin"
binary = "compiler"

input_dir = "snippets"
#input_dir = "kamil_snippets"

dir = os.listdir(main_dir + input_dir)
dir.sort()

error_count = 0

for file_name in dir:
    current_file_args_path = (main_dir + input_dir + '/' + file_name)
    # print(file_name)
    """
    current_file_args = ''
    f = open(current_file_args_path, "r")
    for x in f:
        current_file_args += x
    """
    # vvv this line determines the way the binary is run
    args = (main_dir + binary, current_file_args_path)

    popen = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    popen.wait()
    output = popen.stdout.read()
    returnCode = popen.poll()

    # print(output, '\n---\n')

    """
    if (file_name[0:2] == 'ok' and returnCode != 0) \
            or (file_name[0:3] == 'err' and returnCode != int(file_name[3])):
        error_count += 1
    """
    print('%s : %d' % (file_name, returnCode))

if error_count == 0:
    print('\nAll passed')
else:
    print('\n%d Errors' % (error_count))
