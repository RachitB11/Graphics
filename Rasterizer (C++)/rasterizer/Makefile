OBJS = main.o mat4.o vec4.o raster_tools.o tiny_obj_loader.o
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

rasterize : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o rasterize

main.o : main.cpp raster_tools.h vec4.h mat4.h tiny_obj_loader.h
	$(CC) $(CFLAGS) main.cpp -std=c++11

mat4.o : mat4.h mat4.cpp vec4.h 
	$(CC) $(CFLAGS) mat4.cpp -std=c++11

vec4.o : vec4.h vec4.cpp 
	$(CC) $(CFLAGS) vec4.cpp -std=c++11

raster_tools.o : raster_tools.h raster_tools.cpp vec4.h mat4.h 
	$(CC) $(CFLAGS) raster_tools.cpp -std=c++11

tiny_obj_loader.o : tiny_obj_loader.h tiny_obj_loader.cc
	$(CC) $(CFLAGS) tiny_obj_loader.cc -std=c++11


clean:
	\rm *.o *~ p1
