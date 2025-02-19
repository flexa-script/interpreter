print(0 in {0, 1, 2});
print("\n");
print(1 in {0, 1, 2});
print("\n");
print(2 in {0, 1, 2});
print("\n");
print(-1 in {0, 1, 2});
print("\n");
print(3 in {0, 1, 2});
print("\n");
print(20 in {0, 1, 2});
print("\n");

print({0, 1, 2} in {{0, 1, 2}, {1, 1, 2}, {2, 1, 2}});
print("\n");
print({1, 1, 2} in {{0, 1, 2}, {1, 1, 2}, {2, 1, 2}});
print("\n");
print({2, 1, 2} in {{0, 1, 2}, {1, 1, 2}, {2, 1, 2}});
print("\n");

print({0, 1, 3} in {{0, 1, 2}, {1, 1, 2}, {2, 1, 2}});
print("\n");
print({1, 1, 3} in {{0, 1, 2}, {1, 1, 2}, {2, 1, 2}});
print("\n");
print({2, 1, 3} in {{0, 1, 2}, {1, 1, 2}, {2, 1, 2}});
print("\n");

struct Foo {
	var a: int;
	var s: string;
};

var str1: Foo = Foo{a=10, s="str"};
var str1p = str1;
var str2: Foo = Foo{a=10, s="str"};

// print(str1 == str2);

// has pointer
print(str1 in {Foo{a=1, s="aa"}, str1p, Foo{a=2, s="aa"}});
print("\n");
// has value but no val access
print(str1 in {Foo{a=10, s="str"}, Foo{a=10, s="str"}, Foo{a=10, s="str"}});
print("\n");
// has value
print(unref str1 in {Foo{a=10, s="str"}, Foo{a=10, s="str"}, Foo{a=10, s="str"}});
print("\n");

// strings
print("strings\n\n");
// match
print("match:\n");
print("0" in "012");
print("\n");
print("1" in "012");
print("\n");
print("2" in "012");
print("\n");
print("01" in "012");
print("\n");
print("012" in "012");
print("\n");
print("12" in "012");
print("\n\n");
// dont match
print("dont match:\n");
print("-0" in "012");
print("\n");
print("3" in "012");
print("\n");
print("9" in "012");
print("\n");
print("10" in "012");
print("\n");
print("-0123" in "012");
print("\n");
print("22" in "012");
print("\n\n");

// char
print("chars\n\n");
// match
print("match:\n");
print('0' in "012");
print("\n");
print('1' in "012");
print("\n");
print('2' in "012");
print("\n\n");
// match
print("dont match:\n");
print('\0' in "012");
print("\n");
print('\n' in "012");
print("\n");
print('3' in "012");
print("\n\n");

