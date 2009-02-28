require "mkmf"

cups_cflags = `cups-config --cflags`.chomp
cups_libs = `cups-config --libs`.chomp

with_cflags(cups_cflags) {
  with_ldflags(cups_libs) {
    create_makefile("cups")
  }
}
