from sys import argv

def main(fileName):
    fp = open(fileName, "r")
    lines = fp.readlines()
    fp.close()

    print(lines[0].strip())
    for i in range(1, len(lines)):
        line = lines[i]
        print(line.strip() + " -1 0")

if(len(argv) != 2):
    print("Usage: " + argv[0] + " <file name>")
else:
    main(argv[1])