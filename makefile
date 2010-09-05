
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

OBJ = .Random.o .Actor.o .CircleActor.o .Arena.o .Texture.o .Player.o .Orbitals.o .Particle.o .Collision.o 

${OUT} : ${OBJ} main.cpp makefile
	${link} main.cpp -std=c++0x ${OBJ} ${LDFLAGS}

.Random.o : Random.h Random.cpp
	${compile} Random.cpp -o .Random.o

.Collision.o : Collision.cpp Collision.h makefile
	${compile} Collision.cpp -o .Collision.o

.Actor.o : Actor.cpp Actor.h Collision.h makefile
	${compile} Actor.cpp -o .Actor.o

.CircleActor.o : CircleActor.cpp CircleActor.h .Actor.o .Arena.o
	${compile} CircleActor.cpp -o .CircleActor.o

.Arena.o : Arena.h Arena.cpp
	${compile} Arena.cpp -o .Arena.o

.Player.o : Player.cpp Player.h .CircleActor.o .Texture.o
	${compile} -std=c++0x Player.cpp -o .Player.o

.Orbitals.o : Orbitals.cpp Orbitals.h .CircleActor.o .Texture.o
	${compile} Orbitals.cpp -o .Orbitals.o

.Particle.o : Particle.cpp Particle.h .Actor.o
	${compile} Particle.cpp -o .Particle.o

.Texture.o : Texture.cpp Texture.h
	${compile} Texture.cpp -o .Texture.o

clean:
	rm *.o


