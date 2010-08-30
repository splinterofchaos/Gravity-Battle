
# NOTE:
# LDFLAGS are flags sent to the linker.
# CFLAGS are sent to the C compiler.

ifeq "$(OS)" "Windows_NT"
	LDFLAGS = -lmingw32 -lSDLmain -lSDL -lopengl32
	OUT     = run.exe
else
	# Linux
	LDFLAGS = -lGL -lSDL
	OUT     = run
endif


CC = g++

CFLAGS  += -Wall 

compile = ${CC} ${CFLAGS} -c -Ivector
link    = ${CC} ${CFLAGS} -Ivector -o ${OUT}

${OUT} : .draw_shape.o .glpp.o .Actor.o .Player.o .Collision.o .draw_shape.o main.cpp makefile
	${link} main.cpp -std=c++0x .Actor.o .CircleActor.o .Texture.o .Player.o .Collision.o .draw_shape.o .glpp.o ${LDFLAGS}

.glpp.o : glpp.* makefile
	${compile} glpp.cpp -o .glpp.o

.draw_shape.o : draw_shape.* makefile
	${compile} draw_shape.cpp  -o .draw_shape.o

.Collision.o : Collision.cpp Collision.h makefile
	${compile} Collision.cpp -o .Collision.o

.Actor.o : Actor.cpp Actor.h Collision.h makefile
	${compile} Actor.cpp -o .Actor.o

.CircleActor.o : CircleActor.cpp CircleActor.h .Actor.o
	${compile} CircleActor.cpp -o .CircleActor.o

.Player.o : Player.cpp Player.h .CircleActor.o .Texture.o
	${compile} -std=c++0x Player.cpp -o .Player.o

.Texture.o : Texture.cpp Texture.h
	${compile} Texture.cpp -o .Texture.o

clean:
	rm *.o


