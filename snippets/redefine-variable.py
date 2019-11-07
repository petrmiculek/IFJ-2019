var1 = 8


def bar(arg):
    return arg * 2


def foo():
    var2 = bar(var1)
    var1 = 8.5
    return var1 * var2


foo()
