import os
import sys

class Entry():
    def __init__(self, name, data):
        self.name = name
        self.data = data


if len(sys.argv) <= 1:
    print("Usage arx [output file] [input folder]")
    sys.exit(1)

outf = sys.argv[1]
path = sys.argv[2]

if ~(path.endswith('/') | path.endswith('\\')):
    path += '/'

files = []

for filename in os.listdir(path):
    if(os.path.isfile(path + filename)):
        f = open(path + filename,'rb')
        files.append(Entry(filename,f.read()))
        f.close()


f = open(outf,'wb')
f.write(str.encode('arx'))
for entry in files: 
    f.write(len(entry.name).to_bytes(1,"little"))
    f.write(entry.name.encode())
    f.write(len(entry.data).to_bytes(4,"little"))
    f.write(entry.data)
f.write(int(0).to_bytes(1,"little"))
f.close()
