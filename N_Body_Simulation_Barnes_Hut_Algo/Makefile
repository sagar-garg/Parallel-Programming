NAME = nbody
CC = mpicc
CFLAGS = -std=gnu99 -fstrict-overflow -ggdb -Werror -Wall -Wshadow
LDFLAGS = -lm -lrt -I $(CURDIR)

#error if student directory is not set
ifndef SDIR
SDIR = student
endif

ifndef NOGUI
LDFLAGS += -lX11 -lXrandr -lGL
else
CFLAGS += -DDISABLE_GUI
endif

# The dependency file names.
DEPS := $(OBJ_SEQ:.o=.d)
SRC = *.c $(SDIR)/*.c

all: sequential parallel unit_test

sequential: $(SDIR)/$(NAME)_seq

parallel: $(SDIR)/$(NAME)_par

unit_test: $(SDIR)/unit_test

$(SDIR)/$(NAME)_seq:.FORCE
	$(CC) $(CFLAGS) -DBUILD_SEQ $(SRC) -o $@ $(LDFLAGS)

$(SDIR)/$(NAME)_par: .FORCE
	$(CC) $(CFLAGS) -DBUILD_PAR $(SRC) -o $@ $(LDFLAGS)

$(SDIR)/unit_test: .FORCE
	$(CC) $(CFLAGS) -DBUILD_UNIT $(SRC) -o $@ $(LDFLAGS)

clean:
	rm -f $(SDIR)/$(NAME)_par $(SDIR)/$(NAME)_seq $(SDIR)/unit_test

-include $(DEPS)

.FORCE:
.PHONY: all sequential parallel unit_test clean .FORCE


