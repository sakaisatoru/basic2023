bin_PROGRAMS = tinybasic

AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" -DICONDIR=\"$(datadir)/pixmaps\" -DLOCALEDIR=\"$(localedir)\" -DGETTEXT_PACKAGE=\""$(GETTEXT_PACKAGE)"\"

tinybasic_SOURCES = \
			basic.h	\
			main.c	\
			token.c	\
			editor.c

tinybasic_CFLAGS  =
tinybasic_LDADD   =  $(INTLLIBS)
