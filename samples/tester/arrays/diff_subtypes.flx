fun arr_param1(arr) {
  println(this, "(arr)");
}
fun arr_param2(arr[]) {
  println(this, "(arr[])");
}
fun arr_param3(arr: any) {
  println(this, "(arr: any)");
}
fun arr_param4(arr[]: any) {
  println(this, "(arr[]: any)");
}

var arr1 = {1, 2, 3, 4, 5, 6};
var arr2: any = {1, 2, 3, 4, 5, 6};
var arr3[] = {1, 2, 3, 4, 5, 6};
var arr4[]: any = {1, 2, 3, 4, 5, 6};

println("func 1:");
arr_param1(arr1);
arr_param1(arr2);
arr_param1(arr3);
arr_param1(arr4);

println("func 2:");
arr_param2(arr1);
arr_param2(arr2);
arr_param2(arr3);
arr_param2(arr4);

println("func 3:");
arr_param3(arr1);
arr_param3(arr2);
arr_param3(arr3);
arr_param3(arr4);

println("func 4:");
arr_param4(arr1);
arr_param4(arr2);
arr_param4(arr3);
arr_param4(arr4);
