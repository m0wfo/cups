require "mkmf"

unless have_library("cups") && find_executable("cups-config")
  puts "Couldn't find CUPS libraries on your system. Check they're installed and in your path."
  exit
end

cups_cflags = `cups-config --cflags`.chomp || ""
cups_libs = `cups-config --libs`.chomp || ""

with_cflags(cups_cflags) {
  with_ldflags(cups_libs) {
    create_makefile("cups")
  }
}