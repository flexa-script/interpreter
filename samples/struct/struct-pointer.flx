// struct-pointer.cp

struct Element {
  var has_value : bool;
  var value : any;
};

def element_create(value : any) : Element {
  //var newElement : Element = Element { };
  //newElement.has_value = true; // deveria gerar erro na analise semantica pois newElement é nulo
  //newElement.value = value; // preciso saber se a variável é parametro ou do escopo
  
  var newElement : Element = Element {
    has_value = true,
    value = value // OK - erro pois não aceita expressão
    //value = 0
  };
  //newElement.value = value;
  
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

element1 = element_create(10);

element2 = element1;

element2.value = 20;

element_print(element1);
element_print(element2);

element1.value = 99;

element_print(element1);
element_print(element2);
