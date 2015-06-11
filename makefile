CC        =g++
CFLAGS    =-c -std=gnu++11 -Wall -O3
LDFLAGS   =-lstdc++
INCLUDE   =-I./include/
OBJDIR    =obj/

# Backslash for linebreak. Wohoo!! Yay! (and comment is # obv. Beware, it affects the *whole* line.)
OBJLIST   = Point.o Farm.o Alias_table.o Shipment_kernel.o \
		 	Region.o County.o State.o grid_cell.o Grid_checker.o \
			Grid_manager.o Status_manager.o	shared_functions.o \
			file_manager.o main.o Control_actions.o Shipment_manager.o \
			Local_spread.o

OBJECTS   = $(addprefix $(OBJDIR), $(OBJLIST) )

all:a

a: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJECTS): ./$(OBJDIR)%.o: src/%.cpp
	$(CC) $(CFLAGS) $? -o $@ $(INCLUDE)

clean:
	rm obj/*.o