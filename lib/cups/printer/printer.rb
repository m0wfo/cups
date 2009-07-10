require "cups"

module Cups

  # Right now this just wraps the Cups.options_for hash using method_missing, allowing you to call
  # things like Printer#printer_make_and_model on an instance.
  class Printer

    # Creates a Cups::Printer object. Defaults to the default printer, if there is one.
    def initialize(printer=nil)
      if printer.nil?
        raise "There is no default printer!" if !Cups.default_printer
        @printer = Cups.default_printer
      else
        raise "The printer or destination doesn't exist!" unless Cups.show_destinations.include?(printer)
        @printer = printer
      end
    end

    # Call an options key on this object (with hyphens substituted with underscores). Passes onto super if
    # no key is found or its value is nil.
    def method_missing(m)
      get_options
      key = m.to_s.gsub(/\_/, "-")
      @options[key].nil? ? super : @options[key]
    end

    private

    def get_options
      @options ||= Cups.options_for(@printer)
    end

  end
end
