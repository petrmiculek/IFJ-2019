var1 = 8
arg = 1


def bar(arg):
    pass


def foo():
    var2 = bar(var1)  # this line is still ok
    var1 = 8.5  # gotta throw (semantical error: redefining variable) on this line
    return 0


foo()
