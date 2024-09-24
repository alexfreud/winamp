# win95 specific modules

OBJS = $(OBJS) \
	$(OBJDIR)dxv.$(OBJ) \
    $(OBJDIR)dkprof.$(OBJ) \
	$(OBJDIR)perf.$(OBJ) \
    $(OBJDIR)pentium.$(OBJ) \
	$(OBJDIR)$(PROJECT).res

# modules specifically sent to linker,
# others are provided in libraries

DLLOBJS = $(OBJDIR)dxl_main.obj \
	$(OBJDIR)dxv_mem.$(OBJ) \
	$(OBJDIR)dxv_mems.$(OBJ) \
	$(OBJDIR)$(PROJECT).res \
	$(OBJDIR)$(PROJECT).$(OBJ)

PROJLIBS = $(PROJLIBS) 

#$(OBJDIR)sc_$(PROJECT).lib

