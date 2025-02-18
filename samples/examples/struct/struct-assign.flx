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

def element_print(element : Element) {
  print("Element {\n");
  print("  has_value: " + string(element.has_value));
  print(",\n  value: " + string(element.value));
  print("\n}\n");
}

var element1 : Element;
var element2 : Element;
var element3 : Element;

var element11 : Element;
var element22 : Element;
var element33 : Element;

element1 = element_create(10);

element2.has_value = true;
element2.value = 55;

element11 = element1;
element22 = element2;
element33 = element3;

element_print(element1);
element_print(element11);
element_print(element2);
element_print(element22);
element_print(element3);
element_print(element33);
