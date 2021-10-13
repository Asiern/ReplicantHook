import os
import sys
import getopt
import csv

try:
    opts, args = getopt.getopt(sys.argv[1:], "hf:", ["file"])
except getopt.GetoptError:
    print('csv_to_cpp.py -f <file.csv>')
    sys.exit(2)
for opt, arg in opts:
    if opt == '-h':
        print('csv_to_cpp.py -f <file.csv>')
        sys.exit()
    elif opt in ("-f", "--file"):
        path = arg
    # elif opt in ("-p", "--prefix"):
    #     prefix = arg
    # elif opt in ("-s", "-sufix"):
    #     sufix = arg


f = open("out.cpp", "w")

prefix = 'this->_inventory.insert(std::pair<std::string, uintptr_t>("'
sufix = '));'
with open(path, newline="") as file:
    reader = csv.reader(file, delimiter=",", quotechar="|")
    next(reader)
    for row in reader:
        if  row[0] != "":
            f.write("\n// "+row[0]+"\n")
        f.write(prefix + row[1] + '", ' + row[2] + sufix + "\n")
