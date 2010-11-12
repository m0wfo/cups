Gem::Specification.new do |s|
  s.name = %q{cups}
  s.version = "0.0.7"
  s.authors = ["Chris Mowforth"]
  s.email = ["chris@mowforth.com"]
  s.summary = "A lightweight Ruby library for printing."
  s.description = <<-EOF
    Ruby CUPS provides a wrapper for the Common UNIX Printing System, allowing rubyists to perform basic tasks like printer discovery, job submission & querying.
  EOF
  s.files = Dir.glob("{test}/**/*") | ["ext/cups.c", "ext/ruby_cups.h", "ext/extconf.rb"] | Dir.glob("{lib}/**/*")
  s.extensions << "ext/extconf.rb"
  s.homepage = "http://cups.rubyforge.org/"
  s.has_rdoc = true
  s.rubyforge_project = "cups"
end
