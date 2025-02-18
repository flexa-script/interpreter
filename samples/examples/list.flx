// list.cp

struct Node {
  var value : any;
  var next : Node;
};

struct List {
  var first : Node;
};

def list_add(list : List, value : any) {
  if (not list.first) {
    var newNode : Node = Node {};
    newNode.value = value;
    newNode.next = null;
    list.first = newNode;
  } else {
    var prevNode : Node;// = Node {};
    var currNode = list.first;

    while (currNode) {
      prevNode = currNode;
      currNode = currNode.next;
    }

    var newNode : Node = Node {};
    newNode.value = value;

    currNode.next = newNode;
  }
}

def list_print(list : List) {
  if (not list.first) {
    print("[]");
  } else {
    var node = list.first;
    print("[");
    while (node) {
      print(string(node.value));
      if (node.next) {
        print(",");
      }
      node = node.next;
    }
    print("]");
  }
}

var list : List = List {};

list_add(list, 10);
list_add(list, 9);
list_add(list, 8);
list_add(list, 7);

print(list);
print('\n');
list_print(list);
print("\n\n");


var list2 : List = List {
 first = Node {
   value = 1,
   next = Node {
     value = 2,
     next = Node {
       value = 3,
       next = null
     }
   }
 }
};

print(list2);
print('\n');
list_print(list2);
print("\n\n");
