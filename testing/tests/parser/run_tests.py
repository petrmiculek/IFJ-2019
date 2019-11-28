"""
This runs the binary with all files in input_dir (snippets),
shows its output
and return code (r:_)
"""

import os
import subprocess
import string

main_dir = "../../../"

# binary = "test_bin"
binary = "compiler"

input_dir = "snippets"
dir = os.listdir(main_dir + input_dir)

error_count = 0

for file_name in dir:
    current_file_args = (main_dir + input_dir + '/' + file_name)
    # print(file_name)

    # vvv this line determines the way the binary is run
    args = (main_dir + binary, current_file_args)

    popen = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)
    popen.wait()
    output = popen.stdout.read()
    returnCode = popen.poll()

    # print(output + 'r:', returnCode, '\n---\n')
    # ^^^ print output

    if (file_name[0:2] == 'ok' and returnCode != 0) \
            or (file_name[0:3] == 'err' and returnCode != int(file_name[3])):
        error_count += 1
        print('%s : %d' % (file_name, returnCode))

if error_count == 0:
    print('All passed')
else:
    print('%d Errors' % (error_count))
