require "mkmf"

unless have_library("cups") && find_executable("cups-config")
  puts "Couldn't find CUPS libraries on your system. Check they're installed and in your path."
  exit
end

def has_cups_1_6_installed?
  puts 'cups version:'
  puts `cups-config --version`
  `cups-config --version`.scan(/1.6/).size > 0
end

cups_cflags = `cups-config --cflags`.chomp || ""
cups_cflags += ' -D_IPP_PRIVATE_STRUCTURES' if has_cups_1_6_installed?
cups_libs = `cups-config --libs`.chomp || ""

with_cflags(cups_cflags) {
  with_ldflags(cups_libs) {
    create_makefile("cups")
  }
}