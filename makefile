 
# NOTE:
# LDFLAGS are flags sent to the linker.
# CFLAGS are sent to the C compiler.

CC = g++ 
SDLMAIN = 

ifeq "$(OS)" "Windows_NT"
	# Openmp seems to do more bad than good, so don't include it.
	EXTRA =
	LDFLAGS = -lmingw32 -lSDLmain -lSDL -lSDL_mixer -lopengl32 -lgdi32 -lSDL_ttf
	OUT     = run.exe
else ifeq "${shell uname}" "Darwin"
	LDFLAGS = -framework SDL -framework SDL_mixer -framework SDL_ttf -framework OpenGL -framework cocoa
	#LDFLAGS = -framework OpenGL -framework cocoa
	OUT   = run

	CC = llvm-g++
	SDLMAIN = SDLMain.m
else
	# Linux
	EXTRA =
	LDFLAGS = -lGL -lX11 -lSDL -lSDL_mixer -lSDL_ttf
	OUT     = run
endif

CFLAGS  += -Wall -Wextra -O3

compile = ${CC} ${CFLAGS} ${EXTRA} -c 
link    = ${CC} ${CFLAGS} ${EXTRA} -o ${OUT}

OBJ = .Challenge.o .Config.o .Random.o .Font.o .Actor.o .CircleActor.o .Arena.o .Texture.o .Player.o .Orbitals.o .MenuOrbital.o .Particle.o .Collision.o .Color.o .draw_shape.o .glpp.o .Parsing.o .Sound.o .Timer.o .Keyboard.o

${OUT} : ${OBJ} ${SDLMAIN} main.cpp System.h makefile
	${link} ${SDLMAIN} main.cpp ${OBJ}  ${LDFLAGS}


.Keyboard.o : Keyboard.h Keyboard.cpp
	${compile} Keyboard.cpp -o .Keyboard.o

.Timer.o : Timer.cpp Timer.h
	${compile} Timer.cpp -o .Timer.o

.Sound.o : Sound.h Sound.cpp
	${compile} Sound.cpp -o .Sound.o

.glpp.o : glpp.cpp glpp.h
	${compile} glpp.cpp -o .glpp.o

.draw_shape.o : draw_shape.cpp draw_shape.h
	${compile} draw_shape.cpp -o .draw_shape.o

.Challenge.o : Challenge.h Challenge.cpp Draw.h
	${compile} Challenge.cpp -o .Challenge.o

.Parsing.o : Parsing.cpp Parsing.h
	${compile} Parsing.cpp -o .Parsing.o

.Config.o : Config.h Config.cpp .Parsing.o
	${compile} Config.cpp -o .Config.o

.Random.o : Random.h Random.cpp
	${compile} Random.cpp -o .Random.o

.Font.o : Font.cpp Font.h System.h
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
	${compile} Player.cpp -o .Player.o

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



