struct Element {
  var has_value : bool;
  var value : any;
};

def element_create(value : any) : Element {
  var newElement : Element = Element {};
  newElement.has_value = true;
  newElement.value = value;
  return newElement;
}

var element1 : Element;
var element2 : Element;
var element3 : Element;

var element11 : Element;
var element22 : Element;
var element33 : Element;

element1 = element_create(9);

element2 = Element {};
element2.has_value = true;
element2.value = 99;

element3 = Element {
  has_value = true,
  value = 999
};

element11 = element1;
element22 = element2;
element33 = element3;

print(element1);
print('\n');
print(element11);

print('\n');
print('\n');

print(element2);
print('\n');
print(element22);

print('\n');
print('\n');

print(element3);
print('\n');
print(element33);

print('\n');
print('\n');
