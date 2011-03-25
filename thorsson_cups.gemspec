Gem::Specification.new do |s|
  s.name = %q{thorsson_cups}
  s.version = "0.0.32"
  s.authors = ["Ivan Turkovic", "Chris Mowforth"]
  s.email = ["me@ivanturkovic.com", "chris@mowforth.com"]
  s.summary = "A lightweight Ruby library for printing."
  s.description = <<-EOF
    Ruby CUPS provides a wrapper for the Common UNIX Printing System, allowing rubyists to perform basic tasks like printer discovery, job submission & querying.
  EOF
  s.files = Dir.glob("{test}/**/*") | ["ext/cups.c", "ext/ruby_cups.h", "ext/extconf.rb"] | Dir.glob("{lib}/**/*")
  s.extensions << "ext/extconf.rb"
  s.homepage = "https://github.com/Thorsson/cups"
  s.has_rdoc = true
  s.rubyforge_project = "thorsson_cups"
end