def main():
    fp = open("maps/room.out", "r")
    lines = fp.readlines()
    fp.close()

    for line in lines:
        print(line.strip() + " -1 0")

main()