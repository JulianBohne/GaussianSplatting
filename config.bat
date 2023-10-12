@rem ----- Builtch Configuration -----
@rem --------- Version 0.2.1 ---------

@rem ------------- Files -------------
set source_files=splat.c, comp_shader_util.c, globals_util.c
set output_file=splat.exe

@rem ----------- Arguments -----------
set common_args=-Wall
set debug_args=-D _DEBUG
set release_args=-D NDEBUG -O3
set test_args=-D _DEBUG -D TESTING

@rem ----------- Libraries -----------
call libgan require raylib || exit /b 1

@rem I don't know why, but you have to add this.
@rem Otherwise this doesn't always return 0 when used in cmd.
@rem I think it's fine in other terminals.
exit /b 0 
