var1 = 8


def bar(arg):
    return arg * 2


def foo():
    var2 = bar(var1)  # this line is still ok
    var1 = 8.5  # gotta throw redefining variable err on this line
    return var1 * var2


foo()
