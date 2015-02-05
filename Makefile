all:
	cc hashtable.c -O3 -o hashtable
	time ./hashtable sowpods.csv
	clang++ stdmap.cpp -std=c++11 -stdlib=libc++ -O3 -o stdmap
	time ./stdmap sowpods.csv
