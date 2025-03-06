todo: todo.c html.c html.h

wasm: todo.c html.c html.h
	emcc -o todo.html todo.c html.c -s EXPORTED_FUNCTIONS=_init,_add_todo,ccall --no-entry --shell-file shell.html
