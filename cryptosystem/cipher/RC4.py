import sys


def main(key, infile, outfile):
    f = open(infile, 'rb')
    ff = open(outfile, 'w')
    a = f.read()

    S = list(range(256))
    T = list(range(256))
    j = 0

    for i in range(256):
        S[i] = i
        T[i] = ord(key[i % len(key)])

    # KSA:
    for i in range(256):
        j = (j + S[i] + T[i]) % 256
        S[i], S[j] = S[j], S[i]

    # PRGA:
    res = []
    i = j = 0
    for s in a:
        i = (i + 1) % 256
        j = (j + S[i]) % 256
        S[i], S[j] = S[j], S[i]
        k = S[(S[i] + S[j]) % 256]

        ff.write(chr(s ^ k))

    f.close()
    ff.close()
    return 0


if __name__ == '__main__':
    main(sys.argv[1], sys.argv[2], sys.argv[3])

