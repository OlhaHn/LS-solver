import os
import subprocess
compile_program = 'g++ -std=c++14 -o b.out main.cpp'
os.system(compile_program)

for r, d, f in os.walk('./input'):
    for folder in d:
        print(folder)
        for r, d, f in os.walk('./input/{}'.format(folder)):
            for file_name in f:
                print(file_name)
                fd = open('./input/{}/{}'.format(folder, file_name),  'rb')
                try:
                    result = subprocess.check_output('./b.out', stdin=fd)
                    if folder[0] == 'u' and result.decode('UTF-8') != 'Result: 0\n' :
                        raise Exception("Invalid answer")
                    if folder[0] == 's' and result.decode('UTF-8') != 'Result: 1\n' :
                        raise Exception("Invalid answer")
                    print("ok")
                finally:
                    fd.close()
                    
        