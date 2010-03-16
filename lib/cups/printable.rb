require 'cups'
require 'growl'

module Printable
  attr_reader :print_job
  
  def print!( printer = Cups.default_printer, &block)
    @print_job = Cups::PrintJob.new(self.path, printer)

    # Start a new thread and check for changes in the PrintJob's
    # state. If it gets stopped, cancelled, aborted or completes,
    # run the block passed with #print!
    Thread.new do
      end_states = [:stopped, :cancelled, :aborted, :completed]
      loop do
        state = @print_job.state
        block.call(state) and return if end_states.include?(state)
      end
    end if block_given?
    
    @print_job.print
  end
end