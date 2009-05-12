Gem::Specification.new do |s|
  s.name = %q{cups}
  s.version = "0.0.5"
  s.authors = ["Chris Mowforth"]
  s.email = ["chris@mowforth.com"]
  s.summary = "Print stuff through the Common Unix Printing System using Ruby"
  s.files = Dir.glob("{ext,lib,test}/**/*")
  s.extensions << "ext/extconf.rb"
  s.homepage = "http://cups.rubyforge.org/"
  s.has_rdoc = false
  s.rubyforge_project = "cups"
end
