struct Foo {
  var val1: int;
  var val2: string;
};

def funalt(p: Foo) {
    p.val1 = 99;
}

def funtest(p: Foo) {
    p = null;
}

def funtest(p: int, p1: int, p2: int) {
    p = 1000;
    p1 = 1000;
    p2 = 1000;
}

var v: Foo = Foo{val1=10, val2="aaa"};

print(string(v)+"\n");

funalt(v);

print(string(v)+"\n");

funtest(v);

print(string(v)+"\n");

var i = 10;
var i2 = 10;

print(string(i)+"\n");
print(string(i2)+"\n");

funtest(ref i, 10, i2);

print(string(i)+"\n");
print(string(i2)+"\n");
