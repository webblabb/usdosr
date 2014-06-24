CC        =g++
CFLAGS    =-c -std=gnu++11 -Wall -O3
LDFLAGS   =-lstdc++
INCLUDE   =-I./include/
OBJDIR    =obj/
OBJLIST   = String_functions.o Farm.o grid_cell.o Grid_Creator.o main.o 
OBJECTS   = $(addprefix $(OBJDIR), $(OBJLIST) )

all:a

a: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJECTS): ./$(OBJDIR)%.o: src/%.cpp
	$(CC) $(CFLAGS) $? -o $@ $(INCLUDE)

clean:
	rm obj/*.o