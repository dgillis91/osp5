IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=../obj

_DEPS = semutil.h shmutil.h pclock.h util.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = oss.o shmutil.o semutil.o pclock.o util.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_USER = user.o shmutil.o semutil.o pclock.o util.o
USER = $(patsubst %,$(ODIR)/%,$(_USER))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

oss: $(OBJ)
	$(CC) -o ../bin/$@ $^ $(CFLAGS)

user: $(USER)
	$(CC) -o ../bin/$@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
