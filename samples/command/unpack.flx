using cp.core.exception;
using cp.core.pair;

struct Foo {
	var name: string;
	var age: int;
};

var f = Foo{name="Carlos",age=29};

println("foreach struct as cp::Pair");
foreach(var i: cp::Pair in f){
  println("", i.key, ":", i.value);
}

var u = Foo{name="Nati",age=34};

println("foreach struct as unpack declaration");
foreach(var [key, value] in u){
  println("", key, ":", value);
}


try {
	var a = 10/0;
} catch (var ex: cp::Exception) {
	println("generated error: " + ex.error);
}

try {
	var a = 10/0;
} catch (var [error]) {
	println("generated unpacked error: " + error);
}

try {
	var a = 10/0;
} catch (...) {
	println("error ignored");
}

try {
  throw "cp::Exception error";
} catch (var ex: cp::Exception) {
	println(ex.error);
}

try {
  throw "generated unpacked error";
} catch (var [error]) {
	println(error);
}

try {
  throw "error ignored";
} catch (...) {
	println("error ignored");
}

try {
  throw cp::Exception{error="cp::Exception struct error"};
} catch (var ex: cp::Exception) {
	println(ex.error);
}

try {
  throw cp::Exception{error="generated unpacked struct error"};
} catch (var [error]) {
	println(error);
}

try {
  throw cp::Exception{error="ignored struct error"};
} catch (...) {
	println("error ignored");
}
