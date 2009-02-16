require "cups"

pj = PrintJob.new("/Users/chris/Documents/hash_server.rb", 'soft_class')

# p pj.inspect

# p PrintJob.show_destinations
# p PrintJob.default_printer

# pj.print
p pj.inspect
p pj.job_id