require "cups"

pj = PrintJob.new("/Users/chris/Documents/CV.pdf", "soft_class")
pj.print
p pj.cancel
# p pj.inspect

# p PrintJob.all_jobs
# p PrintJob.default_printer