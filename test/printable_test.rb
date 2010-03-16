#!/usr/bin/env ruby -rubygems

$LOAD_PATH.unshift File.expand_path( File.dirname(__FILE__) + '/../lib')

require 'shoulda'
require 'cups'
require 'cups/printable'
require 'tempfile'

class PrintableTest < Test::Unit::TestCase
  class Object::Tempfile; include Printable end

  context "before calling \#print!" do

    setup { @file = Tempfile.new 'test.txt' }

    should "define \#print!" do
      assert_equal defined?(@file.print!), 'method'
    end
    
    should "return nil for \#print_job" do
      assert_equal defined?(@file.print_job), 'method'
      assert_nil @file.print_job
    end
  end
  
  unless Cups.default_printer
    raise "Can't finish running PrintableTest unless a default printer is set."
  end
  
  context "when calling \#print!" do
    
    setup { @file = Tempfile.new 'test.txt' }
    
    should "print to the default printer... by default" do
      @file.print!
      @file.print_job.cancel
      assert_equal @file.print_job.printer, Cups.default_printer
    end
    
    should "allow passing in a printer" do
      printer = Cups.show_destinations.first
      assert_nothing_raised { @file.print! printer }
      @file.print_job.cancel
      assert_equal @file.print_job.printer, printer
    end
  end
  
  context "after calling \#print!" do
    
    setup { @file = Tempfile.new 'test.txt' }
    
    should "allow access to last print job sent" do
      @file.print!
      @file.print_job.cancel
      assert @file.print_job.is_a?(Cups::PrintJob)
    end
    
    should "run callback after the print_job is cancelled" do
      value = false
      @file.print! { value = true }
      @file.print_job.cancel
      Thread.pass # pass control to Thread polling the job state
      sleep 0.1   # give polling Thread enough time to run callback
      assert value
    end
  end
end