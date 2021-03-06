#Compiler and Linker
CC          := g++

#The Target Binary Program
TARGET      := gl

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := src
INCDIR      := include
BUILDDIR    := obj
TARGETDIR   := bin
RESDIR      := res
SRCEXT      := cpp
DEPEXT      := d
OBJEXT      := o

#Flags, Libraries and Includes

# SDL_CONFIG ?= sdl2-config
# SDL_CFLAGS = $(shell $(SDL_CONFIG) --cflags)
# SDL_LIBS = $(shell $(SDL_CONFIG) --libs)

#PROFILING
#CFLAGS      := -std=c++11 -Wall -O0 -g -pg -ggdb -c
#LFLAGS      := -pg
#DEBUGGING
#CFLAGS      := -std=c++11 -Wall -O0 -g -ggdb -c
#OPTIMIZED
CFLAGS      := -std=c++11 -g -Wall -O3 -c
CFLAGS 		+= $(SDL_CFLAGS)
CFLAGS 		+= $(CURL_CFLAGS)

# LIB	:= $(shell pkg-config --libs $(packages))

LIB 				+= -L$(LD_LIBRARY_PATH) $(SDL_LIBS) $(CURL_LIBS) -lm -lpthread -lGL -lGLU -lglut -ldl -lassimp -lboost_thread -lboost_system `pkg-config --static --libs glfw3`
INC         := -I$(INCDIR) -I$(CPATH) -I/usr/local/include
INCDEP      := -I$(INCDIR)

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------
SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

#Defauilt Make
all: resources $(TARGET)

#Remake
remake: cleaner all

#Copy Resources from Resources Directory to Target Directory
resources: directories
		@cp $(RESDIR)/* $(TARGETDIR)/

#Make the Directories
directories:
		@mkdir -p $(TARGETDIR)
		@mkdir -p $(BUILDDIR)

#Clean only Objecst
clean:
		@$(RM) -rf $(BUILDDIR)
		@$(RM) -rf $(TARGETDIR)

#Full Clean, Objects and Binaries
cleaner: clean
		@$(RM) -rf $(TARGETDIR)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Link

$(TARGET): $(OBJECTS)
		$(CC) $(LFLAGS) -o $(TARGETDIR)/$(TARGET) $^ $(LIB)

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
		@mkdir -p $(dir $@)
		$(CC) $(CFLAGS) $(INC) -c -o $@ $<
		@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
		@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
		@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
		@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
		@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

#Non-File Targets
.PHONY: all remake clean cleaner resources

