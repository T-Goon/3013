all : prolific.c generation.c explorer.c
	gcc prolific.c -o prolific
	gcc generation.c -o generation
	gcc explorer.c -o explorer

prolific : prolific.c
	gcc prolific.c -o prolific

generation : generation.c
	gcc generation.c -o generation

explorer : explorer.c
	gcc explorer.c -o explorer

clean:
	rm prolific generation explorer
