CC          = g++
CFLAGS      = -Wall -std=c++11
#CFLAGS     += -DDEBUG -g
LDFLAGS     = -lproj -lroxml -lyaml-cpp
LINKER      = $(CC) -o
EXECUTABLE  = osm2xmap

SRCDIR      = .
OBJDIR      = obj
BINDIR      = bin

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
RM       = rm -f

all: $(BINDIR)/$(EXECUTABLE)

$(BINDIR)/$(EXECUTABLE): $(OBJECTS)
	@$(LINKER) $@ $(OBJECTS) $(LDFLAGS)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONEY: clean
clean:
	$(RM) $(OBJECTS)
	@echo "Cleanup complete!"

.PHONEY: remove
	remove: clean
	$(RM) $(BINDIR)/$(EXECUTABLE)
	@echo "Executable removed!"

