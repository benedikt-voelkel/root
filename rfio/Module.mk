# Module.mk for rfio module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := rfio
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

RFIODIR      := $(MODDIR)
RFIODIRS     := $(RFIODIR)/src
RFIODIRI     := $(RFIODIR)/inc

##### libRFIO #####
RFIOL        := $(MODDIRI)/LinkDef.h
RFIODS       := $(MODDIRS)/G__RFIO.cxx
RFIODO       := $(RFIODS:.cxx=.o)
RFIODH       := $(RFIODS:.cxx=.h)

RFIOH        := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
RFIOS        := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
RFIOO        := $(RFIOS:.cxx=.o)

RFIODEP      := $(RFIOO:.o=.d) $(RFIODO:.o=.d)

RFIOLIB      := $(LPATH)/libRFIO.$(SOEXT)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(RFIOH))
ALLLIBS     += $(RFIOLIB)

# include all dependency files
INCLUDEFILES += $(RFIODEP)

##### local rules #####
include/%.h:    $(RFIODIRI)/%.h
		cp $< $@

$(RFIOLIB):     $(RFIOO) $(RFIODO) $(ORDER_) $(MAINLIBS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libRFIO.$(SOEXT) $@ "$(RFIOO) $(RFIODO)" \
		   "$(SHIFTLIBDIR) $(SHIFTLIB) $(RFIOLIBEXTRA)"

$(RFIODS):      $(RFIOH) $(RFIOL) $(ROOTCINTTMPEXE)
		@echo "Generating dictionary $@..."
		$(ROOTCINTTMP) -f $@ -c $(RFIOH) $(RFIOL)

all-rfio:       $(RFIOLIB)

map-rfio:       $(RLIBMAP)
		$(RLIBMAP) -r $(ROOTMAP) -l $(RFIOLIB) \
		   -d $(RFIOLIBDEP) -c $(RFIOL)

map::           map-rfio

clean-rfio:
		@rm -f $(RFIOO) $(RFIODO)

clean::         clean-rfio

distclean-rfio: clean-rfio
		@rm -f $(RFIODEP) $(RFIODS) $(RFIODH) $(RFIOLIB)

distclean::     distclean-rfio

##### extra rules ######
ifeq ($(PLATFORM),win32)
$(RFIOO): CXXFLAGS += $(SHIFTCFLAGS) $(SHIFTINCDIR:%=-I%) -DNOGDI -D__INSIDE_CYGWIN__
else
$(RFIOO): CXXFLAGS += $(SHIFTCFLAGS) $(SHIFTINCDIR:%=-I%)
endif
