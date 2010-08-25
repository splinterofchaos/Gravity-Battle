
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

compile = ${CC} ${CFLAGS} -c
link    = ${CC} ${CFLAGS} -o ${OUT}

${OUT} : .draw_shape.o .glpp.o .Playfield.o .Actor.o .Gunman.o .Bullet.o .Collision.o .draw_shape.o main.cpp makefile
	${link} main.cpp -std=c++0x .Playfield.o .Actor.o .Gunman.o .Bullet.o .Collision.o .draw_shape.o .glpp.o ${LDFLAGS}

.glpp.o : glpp.* makefile
	${compile} glpp.cpp -o .glpp.o

.draw_shape.o : draw_shape.* makefile
	${compile} draw_shape.cpp  -o .draw_shape.o

.Collision.o : Collision.cpp Collision.h makefile
	${compile} Collision.cpp -o .Collision.o

.Bullet.o : Bullet.* Actor.h makefile
	${compile} Bullet.cpp -o .Bullet.o

.Gunman.o : Gunman.* Actor.h makefile
	${compile} Gunman.cpp -o .Gunman.o

.Actor.o : Actor.cpp Actor.h Collision.h makefile
	${compile} Actor.cpp -o .Actor.o

.Playfield.o : Playfield.h Playfield.cpp Actor.h Collision.h makefile
	${compile} Playfield.cpp -o .Playfield.o

clean:
	rm *.o


