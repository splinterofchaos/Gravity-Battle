 
# NOTE:
# LDFLAGS are flags sent to the linker.
# CFLAGS are sent to the C compiler.

ifeq "$(OS)" "Windows_NT"
	LDFLAGS = -lmingw32 -lSDLmain -lSDL -lopengl32 -lgdi32
	OUT     = run.exe
else
	# Linux
	LDFLAGS = -lGL -lX11 -lSDL 
	OUT     = run
endif

 
CC = g++ 

CFLAGS  += -Wall 

compile = ${CC} ${CFLAGS} -std=c++0x -c 
link    = ${CC} ${CFLAGS} -std=c++0x -o ${OUT}

OBJ = .Challenge.o .Config.o .Random.o .Font.o .Actor.o .CircleActor.o .Arena.o .Texture.o .Player.o .Orbitals.o .MenuOrbital.o .Particle.o .Collision.o .Color.o .draw_shape.o .glpp.o .Parsing.o

${OUT} : ${OBJ} main.cpp makefile
	${link} main.cpp -std=c++0x ${OBJ} ${LDFLAGS}

.glpp.o : glpp.cpp glpp.h
	${compile} glpp.cpp -o .glpp.o

.draw_shape.o : draw_shape.cpp draw_shape.h
	${compile} draw_shape.cpp -o .draw_shape.o

.Challenge.o : Challenge.h Challenge.cpp Draw.h
	${compile} -std=c++0x Challenge.cpp -o .Challenge.o

.Parsing.o : Parsing.cpp Parsing.h
	${compile} Parsing.cpp -o .Parsing.o

.Config.o : Config.h Config.cpp .Parsing.o
	${compile} Config.cpp -o .Config.o

.Random.o : Random.h Random.cpp
	${compile} Random.cpp -o .Random.o

.Font.o : Font.cpp Font.h
	${compile} Font.cpp -o .Font.o

.Collision.o : Collision.cpp Collision.h 
	${compile} Collision.cpp -o .Collision.o

.Actor.o : Actor.cpp Actor.h Collision.h 
	${compile} Actor.cpp -o .Actor.o

.CircleActor.o : CircleActor.cpp CircleActor.h .Actor.o .Arena.o .Color.o Draw.h
	${compile} CircleActor.cpp -o .CircleActor.o

.Arena.o : Arena.h Arena.cpp
	${compile} Arena.cpp -o .Arena.o

.Player.o : Player.cpp Player.h .CircleActor.o .Texture.o
	${compile} -std=c++0x Player.cpp -o .Player.o

.Orbitals.o : Orbitals.cpp Orbitals.h .CircleActor.o .Texture.o
	${compile} Orbitals.cpp -o .Orbitals.o

.MenuOrbital.o : MenuOrbital.cpp MenuOrbital.h .Orbitals.o
	${compile} MenuOrbital.cpp -o .MenuOrbital.o

.Particle.o : Particle.cpp Particle.h .Actor.o .Random.o
	${compile} Particle.cpp -o .Particle.o

.Texture.o : Texture.cpp Texture.h
	${compile} Texture.cpp -o .Texture.o

.Color.o : Color.cpp Color.h 
	${compile} Color.cpp -o .Color.o

clean:
	rm *.o



