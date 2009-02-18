require "cups"

# pj = PrintJob.new("/Users/chris/Documents/CV.pdf", "soft_class")
# pj.print
# p pj.cancel
# p pj.inspect

# p Cups.show_destinations
p Cups.all_jobs_on(Cups.default_printer)
# p Cups.default_printer