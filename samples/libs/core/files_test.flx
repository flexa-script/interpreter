using cp.core.files;

var file: cp::File = cp::File{};

var res = cp::open(file, "C:\\Users\\Carlos\\repos\\doc.txt", cp::MODE_READ);

if (not res) {
	print("unable to open file");
}
else {
	print(cp::read(file));
	cp::close(file);
}

var file2: cp::File = cp::File{};

var res2 = cp::open(file2, "C:\\Users\\Carlos\\repos\\doc2.txt", cp::MODE_WRITE);

if (not res) {
	print("unable to open file");
}
else {
	cp::write(file2, "this was write using cp files lib");
	cp::close(file2);
}
