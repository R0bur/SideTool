EXE=sidetool.exe
MAP=sidetool.map
LIB=gdi32.lib shell32.lib comctl32.lib
OBJ=sidetool.obj mainwnd.obj iacalc.obj iadisp.obj iaarithm.obj iacalend.obj iactrl.obj debug.obj ..\iadf\iadf.obj
RES=sidetool.res
# Executable file generating options.
EXETYPE=NT
SUBSYSTEM=WINDOWS
# Main rule.
$(EXE): $(OBJ) $(RES)
	del $(EXE)
	link.exe $(OBJ),$(EXE),$(MAP),$(LIB),,$(RES) /EXETYPE:$(EXETYPE) /SUBSYSTEM:$(SUBSYSTEM)
# Clean project
clean:
	del *.exe *.obj *.map *.res
# Implicit rules.
.c.obj:
	dmc.exe $< -c -o$@
.rc.res:
	rcc.exe $< -32 -o$@
