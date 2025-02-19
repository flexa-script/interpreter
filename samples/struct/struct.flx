// struct.cp

struct SubStruct {
  var a : int;
};

struct Test {
  var value : float;
  var i : int;
  var str : string;
  var sub : SubStruct;
};

var test : Test;

test.i = 10;
//test.sub.a = 99; // erro, test.sub não declarado
test.sub = SubStruct {
  a = 99
};

print(test.i);
print('\n');
print(test.sub.a);
print('\n');
