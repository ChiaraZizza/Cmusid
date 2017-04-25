all: cmusid
	LD_LIBRARY_PATH=chromaprint-1.4.2/src ./cmusid 

cmusid: chromaprint
	clang main.c -L chromaprint-1.4.2/src/ -I chromaprint-1.4.2/src/ -lchromaprint -o cmusid

