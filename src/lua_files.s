


	.section .rodata
	.global lua_main
	.type   lua_main, @object
	.align  4
lua_main:
	.incbin "src/lua/main.lua"
lua_main_end:
	.global lua_main_size
	.type   lua_main_size, @object
	.align  4
lua_main_size:
	.int    lua_main_end - lua_main


