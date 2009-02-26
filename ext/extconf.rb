if `which cups-config`.empty?
  warn("Couldn't find cups-config. Do you have libcups installed?")
else
  require "mkmf"
  
  cups_cflags = `cups-config --cflags`.chomp
  cups_libs = `cups-config --libs`.chomp

  with_cflags(cups_cflags) {
    with_ldflags(cups_libs) {
      create_makefile("cups")
    }
  }
end
