// declaration-test

def func_test() {
}
def func_test2(): int {
    return 0;
}
def func_test3(): int[3] {
    return { 1, 2, 3 };
}
def func_test4(): int[3][3] {
    return { { 1, 2, 3 }, { 1, 2, 3 }, { 1, 2, 3 } };
}
def func_test5(): int[3][3][3] {
    return { { { 1, 2, 3 }, { 1, 2, 3 }, { 1, 2, 3 } }, { { 1, 2, 3 }, { 1, 2, 3 }, { 1, 2, 3 } }, { { 1, 2, 3 }, { 1, 2, 3 }, { 1, 2, 3 } } };
}

def func_test6(a, b: int, c: any, d[3], e[3]: int, f[3][3], g[3][3]: int, h[3][3][3], i[3][3][3]: int) {
}

print(func_test3()[1]);
print(func_test4()[1][0]);
print(func_test5()[1][0][2]);
