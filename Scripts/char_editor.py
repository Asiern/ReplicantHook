f = open("./f.txt", "r")
out = open("./out.txt", "w")
for line in f.readlines():
    s = list(line)
    s[5] = '4'
    out.write("".join(s))

    