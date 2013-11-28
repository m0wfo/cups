require "mkmf"

unless have_library("cups") && find_executable("cups-config")
  puts "Couldn't find CUPS libraries on your system. Check they're installed and in your path."
  exit
end

def include_ipp_private_structures?
  puts 'cups version:'
  puts `cups-config --version`
  `cups-config --version`.scan(/1.(6|7)/).size > 0
end

cups_cflags = `cups-config --cflags`.chomp || ""
cups_cflags += ' -D_IPP_PRIVATE_STRUCTURES' if include_ipp_private_structures?
cups_libs = `cups-config --libs`.chomp || ""

with_cflags(cups_cflags) {
  with_ldflags(cups_libs) {
    create_makefile("cups")
  }
}
