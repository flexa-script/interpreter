using cp.core.pair;

struct Bar {
	var x: int;
	var j: bool;
};

struct Foo {
	var name: string;
	var age: int;
	var bar: bool;
};

var f = Foo{name="Carlos",age=29,bar=Bar{x=10,j=false}};
// var f = Foo{name="Carlos", age = 29, bar = null};

foreach (var it in f) {
	print("key: ", it.key, "\nvalue: ", it.value, "\ntype: ", typeof(it), '\n');
}
