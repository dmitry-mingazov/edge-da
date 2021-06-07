CC = gcc
SRCDIR = src
SRC = $(wildcard $(SRCDIR)/*.c)
CLEAN = rm -f
CLEANDIR = rm -fd
MKDIR = mkdir -p

OUTDIR = out
OUTS = $(patsubst $(SRCDIR)/%.c, $(OUTDIR)/%, $(SRC))

CCFLAGS = -Wall -g
ZMQ = -lzmq
UUID = -luuid



all: $(OUTDIR) $(OUTS)

$(OUTDIR)/%: $(SRCDIR)/%.c
	$(CC) $(CCFLAGS) $< $(UUID) $(ZMQ) -o $@

$(OUTDIR):
	$(MKDIR) $(OUTDIR)

clean:
	$(CLEAN) $(OBJDIR)/*.o
