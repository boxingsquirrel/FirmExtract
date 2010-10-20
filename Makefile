LINCC=gcc
WINCC=i586-mingw32msvc-gcc

# Fall back on the default gcc...
CC=$(LINCC)
CP=cp
RM=rm -rf
MKDIR=mkdir

CFLAGS=-std=gnu99 `pkg-config --cflags gtk+-2.0`
LFLAGS=`pkg-config --libs gtk+-2.0` -lzip -lssl -lplist

SRCDIR=src
RESDIR=res

SRCFILES=$(SRCDIR)/gtk-ipsw.c $(SRCDIR)/file.c $(SRCDIR)/ui.c $(SRCDIR)/decrypt_root_fs.c $(SRCDIR)/ipsw.c $(SRCDIR)/decrypt_img3.c $(SRCDIR)/xpwntool/abstractfile.c $(SRCDIR)/xpwntool/img3.c $(SRCDIR)/xpwntool/nor_files.c $(SRCDIR)/xpwntool/8900.c $(SRCDIR)/xpwntool/img2.c $(SRCDIR)/xpwntool/ibootim.c $(SRCDIR)/xpwntool/lzss.c $(SRCDIR)/xpwntool/lzssfile.c $(SRCDIR)/xpwntool/libxpwn.c
OBJFILES=$(subst .c,.o,$(SRCFILES))
MENUFILE=ipswTool.desktop

OUTFILE=firmextract

INSTPATH=/usr/bin
DATAPATH=/usr/share/ipswTool

all: tool
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

setccwin:
	CC=$(WINCC)

windows: setccwin tool

tool: $(OBJFILES)
	$(CC) -o $(OUTFILE) $(OBJFILES) $(LFLAGS)
	gtk-builder-convert $(SRCDIR)/ui.glade $(RESDIR)/ui.xml
	gtk-builder-convert $(SRCDIR)/about.glade $(RESDIR)/about.xml
	@echo ""
	@echo "Run 'make install' as root to install"

install:
	@echo "Installing 1 file to "$(INSTPATH)"..."
	$(CP) ./$(OUTFILE) $(INSTPATH)/$(OUTFILE)
	@echo "Installing 4 files to "$(DATAPATH)"..."
	$(RM) $(DATAPATH)
	$(MKDIR) $(DATAPATH)
	$(MKDIR) $(DATAPATH)/res $(DATAPATH)/data $(DATAPATH)/scripts
	$(CP) ./res/* $(DATAPATH)/res
	$(CP) ./data/* $(DATAPATH)/data
	$(CP) ./scripts/* $(DATAPATH)/scripts
	@echo "Adding "$(OUTFILE)" to the menu..."
	cp $(MENUFILE) /usr/share/applications/$(MENUFILE)
	@echo "Install complete"
	@echo "To remove run 'make uninstall' as root"

uninstall:
	@echo "Removing 1 file from "$(INSTPATH)"..."
	$(RM) $(INSTPATH)/$(OUTFILE)
	@echo "Removing 4 files from "$(DATAPATH)"..."
	$(RM) $(DATAPATH)
	@echo "Removing "$(OUTFILE)" from the menu..."
	$(RM) /usr/share/applications/$(MENUFILE)
	@echo "Uninstall complete"

clean:
	$(RM) $(OUTFILE) $(OBJFILES)
