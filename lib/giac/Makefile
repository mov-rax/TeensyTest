CXX	= sh3eb-elf-g++
AR	= sh3eb-elf-gcc-ar
RANLIB	= sh3eb-elf-gcc-ranlib

RM	= /bin/rm

CXXFLAGS = -Os -mb -m4a-nofpu  -mhitachi -std=c++11 -fpermissive -flto -fno-use-cxa-atexit -fno-strict-aliasing -fno-rtti -fno-exceptions -DHAVE_CONFIG_H -DTIMEOUT -DRELEASE  -I. -I/home/parisse/casiolocal/include/ustl -DFILEICON # -DNO_TEMPLATE_MULTGCD -DGIAC_GENERIC_CONSTANTS -fno-zero-initialized-in-bss -ffunction-sections -fdata-sections 

LDFLAGS = -static -nostdlib -Tprizm.ld

LIBS = -L. -L/home/parisse/casiolocal/lib -Wl,--start-group -lgui -lcas -ltommath -lustl -lm -lc -lgcc -Wl,--end-group

CAS_OBJS = ksym2poly.o kgausspol.o kthreaded.o kcsturm.o kmaple.o krpn.o kmoyal.o kmisc.o kpermu.o kdesolve.o input_parser.o ksymbolic.o index.o kmodpoly.o kmodfactor.o kezgcd.o kderive.o ksolve.o kintg.o kintgab.o klin.o kseries.o ksubst.o kvecteur.o kglobal.o kifactor.o kalg_ext.o kgauss.o khelp.o kti89.o kplot.o kprog.o kunary.o kusual.o kidentificateur.o kgen.o krisch.o input_lexer.o first.o # memmgr.o mem.o

# old console: kmisc.cc should be compiled with -DOLD_CONSOLE
#GUI_OBJS = fileGUI.o inputGUI.o menuGUI.o textGUI.o fileProvider.o graphicsProvider.o stringsProvider.o history.o kdisplay.o dConsole.o dmain.o
# new console
GUI_OBJS = fileGUI.o menuGUI.o textGUI.o fileProvider.o graphicsProvider.o stringsProvider.o kdisplay.o console.o main.o

.PRECIOUS: libcas.a libgui.a

all: khicas.g3a khicasen.g3a # pour la version console, supprimer le lien iostream -> iostream.new

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

libgui.a: $(GUI_OBJS)
	$(RM) -f $@
	$(AR) cru $@ $^
	$(RANLIB) $@

libcas.a: $(CAS_OBJS)
	$(RM) -f $@
	$(AR) cru $@ $^
	$(RANLIB) $@

khicas.bin: libcas.a libgui.a catalogfr.o 
	$(CXX) $(LDFLAGS) catalogfr.o $(LIBS) -o $@

khicasen.bin: libcas.a libgui.a catalogen.o 
	$(CXX) $(LDFLAGS) catalogen.o  $(LIBS) -o $@

khicas.g3a: khicas.bin
	mkg3a -n basic:Khicas -n internal:KHICAS -V 1.5.1 -i uns:khicasio.png -i sel:khicasio1.png $^ $@

khicasen.g3a: khicasen.bin
	mkg3a -n basic:Khicasen -n internal:KHICASEN -V 1.5.1 -i uns:khicasio.png -i sel:khicasio1.png $^ $@

clean:
	$(RM) -f *.o libcas.a libgui.a 
