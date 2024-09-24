
$(OBJDIR)sc_$(PROJECT).lib: $(LIBDIR)\sc_tm20.lib $(LIBDIR)\sc_torq.lib
    LIB $(OBJS) /OUT:$@
    LIB $@ $(LIBDIR)\s_tm1.lib /OUT:$@
    LIB $@ $(LIBDIR)\sc_tm20.lib /OUT:$@
    LIB $@ $(LIBDIR)\s_tmrt.lib /OUT:$@
    LIB $@ $(LIBDIR)\sc_torq.lib /OUT:$@
    copy $@ $(LIBDIR)

