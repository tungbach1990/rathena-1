
HTTPLIB_OBJ =  $(shell find * -type f -name "*.cc" | sed -e "s/\.cc/\.o/g")
HTTPLIB_DIR_OBJ = $(HTTPLIB_OBJ:%=obj/%)
HTTPLIB_H =  $(shell find * -type f -name "*.h")
HTTPLIB_AR = obj/httplib.a

CXXFLAG =-std=c++11

WANTS_WEBSERVICE=yes
ifeq ($(WANTS_WEBSERVICE),yes)
	HTTPLIB_DEPENDS=httplib
else
	HTTPLIB_DEPENDS=skip
endif




#####################################################################
.PHONY : all clean help httplib

all: $(HTTPLIB_DEPENDS)

clean:
	@echo "	CLEAN	httplib"
	@rm -rf obj *.o

help:
	@echo "possible targets are 'all' 'clean' 'help'"
	@echo "'httplib' - build $(HTTPLIB_AR)
	@echo "'all'       - builds $(HTTPLIB_DIR_OBJ)"
	@echo "'clean'     - deletes $(HTTPLIB_DIR_OBJ)"
	@echo "'help'      - outputs this message"

#####################################################################

skip:
	@echo "Skipping web-service build, not requested"
	@exit 0

obj:
	@echo "	MKDIR	obj"
	@mkdir -p obj
	
obj/%.o: %.cc $(HTTPLIB_H)
	@echo "	CXX	$<"
	@g++ $(CXXFLAG) -flto -fuse-linker-plugin -ffat-lto-objects -flto -fuse-linker-plugin -g -O2 -pipe -ffast-math -Wall -Wempty-body -Wno-switch -Wno-missing-field-initializers -Wno-maybe-uninitialized -Wno-clobbered -Wshadow  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_SETRLIMIT -DHAVE_STRNLEN  -I/usr/include -DHAVE_MONOTONIC_CLOCK -g $(INC) -c $(OUTPUT_OPTION) $<

httplib: obj $(HTTPLIB_DIR_OBJ) $(HTTPLIB_AR)

$(HTTPLIB_AR): $(HTTPLIB_DIR_OBJ)
	@echo "	AR	$@"
	@/bin/ar rcs obj/httplib.a $(HTTPLIB_DIR_OBJ)
