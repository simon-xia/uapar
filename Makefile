UP_FLAGS = -O2 -g
UP_OBJS = up_ip4.o up_hash.o up_log.o up_darray.o\
	up_interface.o up_path.o up_subnet.o uapar.o up_load_paths.o up_alias.o

uapar: $(UP_OBJS)
	cc -o uapar $(UP_OBJS) $(UP_FLAGS)

$(UP_OBJS) : up_common.h


# unit test
.PHONY:test
test: ip4_test hash_test darray_test interface_test load_paths_test log_test alias_test
ip4_test: up_ip4.c up_ip4.h
	cc -o ip4_test up_ip4.c -D UNIT_TEST_IP4 $(UP_FLAGS)

hash_test: up_hash.c up_ip4.c up_path.c up_interface.c up_log.c up_darray.c
	cc -o hash_test up_hash.c up_ip4.c up_path.c up_log.c up_darray.c up_interface.c -D UNIT_TEST_HASH $(UP_FLAGS)

darray_test: up_log.c up_interface.c up_path.c up_ip4.c up_darray.c 
	cc -o darray_test up_log.c up_interface.c up_path.c up_ip4.c up_darray.c -D UNIT_TEST_D_ARRAY $(UP_FLAGS)

interface_test: up_interface.c up_path.c
	cc -o interface_test up_darray.c up_log.c up_interface.c up_ip4.c up_path.c -D UNIT_TEST_INTERFACE $(UP_FLAGS)

load_paths_test: up_load_paths.c up_subnet.c up_ip4.c up_path.c up_interface.c up_darray.c up_log.c up_hash.c
	cc -o load_paths_test up_load_paths.c up_subnet.c up_ip4.c up_path.c up_darray.c up_log.c up_hash.c up_interface.c -D UNIT_TEST_LOAD_PATHS $(UP_FLAGS)

alias_test: up_alias.c up_load_paths.c up_subnet.c up_ip4.c up_path.c up_interface.c up_darray.c up_log.c up_hash.c
	cc -o alias_test up_alias.c up_load_paths.c up_subnet.c up_ip4.c up_path.c up_darray.c up_log.c up_hash.c up_interface.c -D UNIT_TEST_ALIAS $(UP_FLAGS)

log_test: up_log.c
	cc -o log_test up_log.c -D UNIT_TEST_LOG $(UP_FLAGS)


.PHONY:clean cleantest cleanobj
clean: cleanobj cleantest
	rm uapar 

cleantest:
	-rm ip4_test hash_test darray_test interface_test load_paths_test log_test

cleanobj:
	-rm *.o
