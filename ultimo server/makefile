#variable compilador
CC=g++


CFLAGS=-c

all:server client serverPartida

server: server.o socketStream.o semaphoreHelper.o serverTorneo.o player.o threadMutexHelper.o
	$(CC) -o server server.o socketStream.o semaphoreHelper.o serverTorneo.o player.o threadMutexHelper.o -pthread
serverPartida: serverPartida.o socketStream.o semaphoreHelper.o threadMutexHelper.o player.o serverPartidaTorneo.o cGame.o cPlayer.o cBarrel.o cDonkeyKong.o cFlame.o cPauline.o cOil.o cSprite.o cTextFactory.o md5.o
	$(CC) -o serverPartida serverPartida.o socketStream.o semaphoreHelper.o threadMutexHelper.o player.o serverPartidaTorneo.o cGame.o cPlayer.o cBarrel.o cDonkeyKong.o cFlame.o cPauline.o cOil.o cSprite.o cTextFactory.o md5.o -pthread -lSDL -lSDL_image -lSDL_ttf
client: client.o socketStream.o clientTorneo.o threadMutexHelper.o
	$(CC) -o client client.o socketStream.o clientTorneo.o threadMutexHelper.o -pthread

server.o:server.cpp
	$(CC) $(CFLAGS) server.cpp
client.o:client.cpp
	$(CC) $(CFLAGS) client.cpp

#LIBS
player.o: ./player/player.hpp ./player/player.cpp
	$(CC) $(CFLAGS) ./player/player.cpp

serverTorneo.o: ./serverTorneo/serverTorneo.hpp ./serverTorneo/serverTorneo.cpp
	$(CC) $(CFLAGS) ./serverTorneo/serverTorneo.cpp

clientTorneo.o: ./clientTorneo/clientTorneo.hpp ./clientTorneo/clientTorneo.cpp
	$(CC) $(CFLAGS) ./clientTorneo/clientTorneo.cpp

#Clases Lucas
cGame.o: ./Clases/cGame.h ./Clases/cGame.cpp
	$(CC) $(CFLAGS) ./Clases/cGame.cpp

cPlayer.o: ./Clases/cPlayer.h ./Clases/cPlayer.cpp
	$(CC) $(CFLAGS) ./Clases/cPlayer.cpp

cBarrel.o: ./Clases/cBarrel.h ./Clases/cBarrel.cpp
	$(CC) $(CFLAGS) ./Clases/cBarrel.cpp

cOil.o: ./Clases/cOil.h ./Clases/cOil.cpp
	$(CC) $(CFLAGS) ./Clases/cOil.cpp

cDonkeyKong.o: ./Clases/cDonkeyKong.h ./Clases/cDonkeyKong.cpp
	$(CC) $(CFLAGS) ./Clases/cDonkeyKong.cpp

cFlame.o: ./Clases/cFlame.h ./Clases/cFlame.cpp
	$(CC) $(CFLAGS) ./Clases/cFlame.cpp

cPauline.o: ./Clases/cPauline.h ./Clases/cPauline.cpp
	$(CC) $(CFLAGS) ./Clases/cPauline.cpp

cSprite.o: ./Clases/cSprite.h ./Clases/cSprite.cpp
	$(CC) $(CFLAGS) ./Clases/cSprite.cpp

cTextFactory.o: ./Clases/cTextFactory.h ./Clases/cTextFactory.cpp
	$(CC) $(CFLAGS) ./Clases/cTextFactory.cpp

md5.o: ./Clases/md5.h ./Clases/md5.cpp
	$(CC) $(CFLAGS) ./Clases/md5.cpp

#HELPERS
socketStream.o: ./helpers/socketStream.hpp ./helpers/socketStream.cpp
	$(CC) $(CFLAGS) ./helpers/socketStream.cpp

semaphoreHelper.o: ./helpers/semaphoreHelper.hpp ./helpers/semaphoreHelper.cpp
	$(CC) $(CFLAGS) ./helpers/semaphoreHelper.cpp

threadMutexHelper.o: ./helpers/threadMutexHelper.hpp ./helpers/threadMutexHelper.cpp
	$(CC) $(CFLAGS) ./helpers/threadMutexHelper.cpp

serverPartidaTorneo.o: ./serverPartidaTorneo/serverPartidaTorneo.hpp ./serverPartidaTorneo/serverPartidaTorneo.cpp
	$(CC) $(CFLAGS) ./serverPartidaTorneo/serverPartidaTorneo.cpp

clean:
	rm -f *.o *.a server client serverPartida
