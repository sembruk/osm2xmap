#
#    Copyright 2016 Semyon Yakimov
#
#    This file is part of Osm2xmap.
#
#    Osm2xmap is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    Osm2xmap is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Osm2xmap.  If not, see <http://www.gnu.org/licenses/>.

GIT_VERSION = $(shell git describe --abbrev=4 --tags)
GIT_TIMESTAMP = $(shell git log -n 1 --format=%ai)

CC          = g++
CFLAGS      = -Wall -std=c++11 -DVERSION_STRING='"$(GIT_VERSION) ($(GIT_TIMESTAMP))"'
#CFLAGS     += -DDEBUG -g
LDFLAGS     = -lproj -lroxml -lyaml-cpp
LINKER      = $(CC) -o

LIBDIRS     = .
INCLUDEDIRS = .

EXECUTABLE  = osm2xmap

SRCDIR      = src
OBJDIR      = src
BINDIR      = .

SOURCES  = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
RM       = rm -f

all: $(BINDIR)/$(EXECUTABLE)

$(BINDIR)/$(EXECUTABLE): $(OBJECTS)
	@$(LINKER) $@ $(OBJECTS) $(LDFLAGS) $(foreach d, $(LIBDIRS), -L$d)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@$(CC) $(CFLAGS) $(foreach d, $(INCLUDEDIRS), -I$d) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONEY: clean
clean:
	$(RM) $(OBJECTS)
	@echo "Cleanup complete!"

.PHONEY: remove
remove: clean
	$(RM) $(BINDIR)/$(EXECUTABLE)
	@echo "Executable removed!"

