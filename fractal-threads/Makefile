CC= gcc
GFX= gfx.c
TFLAG= -pthread
GFLAGS1= -lX11
GFLAGS2= -lm
CFLAGS= -std=c99


all: fractalthread fractal fractaltask

fractalthread: fractalthread.c
	$(CC) $(CFLAGS) $(TFLAG) fractalthread.c $(GFX) $(GFLAGS1) $(GFLAGS2) -o fractalthread

fractal: fractal.c
	$(CC) $(CFLAGS) $(TFLAG) fractal.c $(GFX) $(GFLAGS1) $(GFLAGS2) -o fractal

fractaltask: fractaltask.c
	$(CC) $(CFLAGS) $(TFLAG) fractaltask.c $(GFX) $(GFLAGS1) $(GFLAGS2) -o fractaltask

clean:
	rm -f *.o
