all: ls

ls: main.c
	gcc $< -o $@
gdb: main.c
	gcc -g $< -o $@
clean: test
	rm $<

run: test
	./$<
