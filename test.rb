require "cups"

pj = PrintJob.new("/Users/chris/Documents/CV.pdf", "soft_class")
pj.print
p pj.cancel