FFMPEG_LIBS = libavutil libavcodec libavdevice libavformat

a.out: main.o
	gcc $^ $(shell pkg-config --libs $(FFMPEG_LIBS))

main.o: main.c
	gcc -c $< $(shell pkg-config --cflags $(FFMPEG_LIBS))
