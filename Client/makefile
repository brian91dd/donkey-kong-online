################
# CONFIGURACION
################

# Nombre del ejecutable.
target = Client

# Extensión de los archivos a compilar (c para C, cpp o cc o cxx para C++).
extension = cpp

###########################
# CONFIGURACION "AVANZADA"
###########################

# Opciones para el compilador C/C++ en modo ansi.
CFLAGS = -Wall -pedantic -pedantic-errors

# Para que explote lo mas posible
CFLAGS += -O3 -DNDEBUG

# Para valgrind o debug
#CFLAGS += -ggdb -DDEBUG -fno-inline

# Opciones del enlazador.
#LDFLAGS =

# Estandar de C a usar
CSTD = c99

# Estandar de C++ a usar
CXXSTD = c++98

# Bibliotecas a linkear
LOADLIBS = -lpthread -lSDLmain -lSDL -lSDL_image -lSDL_ttf

####################################################
# VARIABLES CALCULADAS A PARTIR DE LA CONFIGURACION
####################################################

# Linkeo con pthreads de ser necesario.
ifdef pthreads
LDFLAGS += -lpthread -pthread
endif

ifdef static
LDFLAGS += -static
endif

# Pongo flags de C para C++ también
CXXFLAGS += $(CFLAGS)

# Uso enlazador de c++ si es código no C.
ifeq ($(extension), c)
CFLAGS += -std=$(CSTD)
enlazador = $(CC)
else
CXXFLAGS += -std=$(CXXSTD)
enlazador = $(CXX)
endif

# Si no especifica archivos, tomo todos.
fuentes ?= $(wildcard *.$(extension))

# REGLAS
#########

.PHONY: all clean

all: $(target)

o_files = $(patsubst %.$(extension),%.o,$(fuentes))

$(target): $(o_files)
	@if [ -z "$(o_files)" ]; \
	then \
		echo "No hay archivos de entrada, recuerde que la extensión debe ser '.$(extension)'."; \
		false; \
	fi >&2
	$(enlazador) $(LDFLAGS) $(o_files) $(LOADLIBS) $(LDLIBS) -o $(target)

clean:
	@$(RM) -fv *.o $(target)

