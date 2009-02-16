require "cups"

pj = PrintJob.new("/Users/chris/Documents/hash_server.rb", "PDF_Printer")

p pj.inspect

pj.print