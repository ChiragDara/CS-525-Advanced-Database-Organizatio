output1: test_assign3_1.o dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o
	gcc -g test_assign3_1.o dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o buffer_mgr.o -lm buffer_mgr_stat.o -o output1

test_expr: test_expr.o dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o
	gcc -g test_expr.o dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o -o test_expr

dberror.o: dberror.c
	gcc -c dberror.c

test_expr.o: test_expr.c
	gcc -c test_expr.c

buffer_mgr_stat.o: buffer_mgr_stat.c
	gcc -c buffer_mgr_stat.c

expr.o: expr.c
	gcc -c expr.c

record_mgr.o: record_mgr.c
	gcc -c record_mgr.c

rm_serializer.o: rm_serializer.c
	gcc -c rm_serializer.c

storage_mgr.o: storage_mgr.c
	gcc -c storage_mgr.c

buffer_mgr_o: buffer_mgr.c
	gcc -c buffer_mgr.c

clean:
	rm *.o *.bin output1 test_expr