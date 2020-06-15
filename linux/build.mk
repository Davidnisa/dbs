PROJECT		:= dbs
TARGET		:= dbs
ROOTDIR		:= ./
OUTDIR		:= ./bin/
INCDIR		:= ../code
SRCDIR		:= ../code
LIBDIR		:= ./bin/lib
DEBUG		:= YES
CCFLAGS		:= \
		-DELF_HAVE_PRAGMA_ONCE \
		-DELF_USE_ALL

CFLAGS		:= \
	-I$(INCDIR) \
	-I$(INCDIR)/pb \
	-I/usr/include/mysql

SRCDIRS		:= ../code
LIBS		:= \
	-lpthread \
	-lprotobuf \
	-lmysqlclient \
	-lpq \
	-lsqlite3


LDFLAGS		:= \
	-L/usr/local/freetds/lib 	\
	-L$(LIBDIR) \
	$(LIBS)



CC := g++
SRCS_C_EXCLUDE_FILTER 	:=
SRCS_CPP_EXCLUDE_FILTER	:=

ifeq (YES, $(DEBUG))
	TARGET		:= $(OUTDIR)/$(TARGET)_d
	CCFLAGS		+= -Wall -Wno-format -Wno-write-strings -g -std=c++11
	OBJDIR		:= .obj/debug
else
	TARGET		:= $(OUTDIR)/$(TARGET)
	CCFLAGS		+= -Wall -Wno-unknown-pragmas -Wno-write-strings -Wno-format -O3 -std=c++11
	OBJDIR		:= .obj/release
endif

RM := rm -rf

INCS		:= $(subst $(INCDIR)/,,$(shell find $(INCDIR) \
	-name '*.h' -print))

SRCS_CC		:= $(shell find $(SRCDIRS) \
	$(SRCS_C_EXCLUDE_FILTER) \
	-name '*.cc' -print)

SRCS_C		:= $(shell find $(SRCDIRS) \
	$(SRCS_C_EXCLUDE_FILTER) \
	-name '*.c' -print)

OBJS_CC     := $(SRCS_CC:$(SRCDIR)/%.cc=$(OBJDIR)/%.o)

OBJS_C     := $(SRCS_C:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS_CC) $(OBJS_C)
	$(CC) -o $@ $(OBJS_CC) $(OBJS_C) $(CCFLAGS) $(LDFLAGS) "-Wl,-rpath,../lib:/usr/local/lib/"
	@echo
	@echo Compile/Link '$(TARGET)' ... OK
	@echo

$(OBJS_CC): $(OBJDIR)%.o: $(SRCDIR)%.cc
	@echo Compiling: $<
	@[ -d $(@D) ] || mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CCFLAGS) -c $< -o $@

$(OBJS_C): $(OBJDIR)%.o: $(SRCDIR)%.c
	@echo Compiling: $<
	@[ -d $(@D) ] || mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CCFLAGS) -c $< -o $@

clean:
	@$(RM) $(OBJDIR)
	@$(RM) $(OUTDIR)/$(TARGET)
	@echo
	@echo Clean '$(TARGET)' ... OK
	@echo