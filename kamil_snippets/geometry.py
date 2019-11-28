print("GEOMETRY CALCULATIONS")
exit = 0


def ProcessShape(shape):
    if (shape == "square"):
        ProcessSquare()
    else:
        if (shape == "ractangle"):
            ProcessRactangle()
        else:
            if (shape == "circle"):
                ProcessCircle()
            else:
                if (shape == "triangle"):
                    ProcessTriangle()
                else:
                    return 1
    return 0


def ProcessSquare():
    print("a = ")
    a = inputf()
    o = 4 * a
    s = a * a
    print("o = ", o, "\ns = ", s)


def ProcessRectangle():
    print("a = ")
    a = inputf()
    print("b = ")
    b = inputf()
    o = 2 * a + 2 * b
    s = a * b
    print("o = ", o, "\ns = ", s)


def ProcessCircle():
    print("r = ")
    r = inputf()
    o = 2 * PI * r
    s = PI * r * r
    print("o = ", o, "\ns = ", s)


def ProcessTrangle():
    print("a = ")
    a = inputf()
    print("b = ")
    b = inputf()
    print("c = ")
    c = inputf()
    print("va = ")
    va = inputf()
    o = a + b + c
    s = a * va / 2
    print("o = ", o, "\ns = ", s)


while (exit):
    print("Select shape (square, ractangle, circle, triangle): ")
    shape = inputs()
    exit = ProcessShape(shape)

print("PROGRAM EXITING")
