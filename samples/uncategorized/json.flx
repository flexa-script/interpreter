// using cp.core.pair;

struct Bar {
	var x: int;
	var j: bool;
};

struct Foo {
	var name: string;
	var age: int;
	// var bar: bool;
	var bar: Bar;
};

var f = Foo{name="Carlos",age=29,bar=Bar{x=10,j=false}};

def substr(str, s, n = null){
	// print("called " + this, '\n');
	var ss = "";
	if(n == null) {
		n = len(str);
	}
	for(var i = s; i < n; i++){
		ss += str[i];
	}
	return ss;
}

def to_json(f){
	// print("called " + this, '\n');
	var json = "{";
	foreach (var it in f) {
		println(it);
		json += '"' + it.key + "\":";
		if(typeof(it.value)=="Bar"){
			json += to_json(it.value);
		}
		else{
			// print("key: ", it.key, "\nvalue: ", it.value, "\ntype: ", typeof(it), '\n');
			json += '"' + string(it.value) + '"';
		}
		json += ",";
		// print(json, '\n');
	}
	// print("json depois : ",json, '\n');
	if(json != "{") {
		json = substr(json, 0, len(json)-1);
	}
	json += '}';
	return json;
}

print(to_json(f));
