require "cups"

class Cups
  class << self
    def active_jobs(printer=default_printer)
      jobs_on(printer, 1, 0)
    end
  end
end

# pj = PrintJob.new("/Users/chris/Documents/CV.pdf", "soft_class")
# pj.print
# p pj.cancel
# p pj.inspect

# p Cups.show_destinations
p Cups
# p Cups.active_jobs
# p Cups.default_printer