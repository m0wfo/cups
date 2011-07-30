require "cups"
require "tempfile"

module Cups

  class PrintJob

    # Unlike its superclass, a Transient object takes a string of data (e.g. from an IO object).
    # This is useful when you've got something like a PDF, an ImageMagick byte array, etc that you
    # just want to send on its way rather than having to touch the file system directly.
    # === A contrived example:
    # Say we're using Prawn[http://prawn.majesticseacreature.com/] to generate PDF invoices for some nebulous,
    # ever-expanding system that promises us sex, drugs, rock & roll, fame and perhaps a pay-rise. Instead of just
    # generating a file, let's pass the rendered version to a Transient object and let it take care of things:
    #
    # ---
    #
    #   require 'cups/print_job/transient'
    #   require 'prawn'
    #
    #   invoice = Prawn::Document.new do
    #     text "Invoice."
    #   end
    #
    #   paper_copy = Cups::PrintJob::Transient.new(invoice.render)
    #   paper_copy.print
    #
    # ---
    #
    # As of 0.0.5, all instance methods inherited from PrintJob are unaltered. This may change if I need to
    # delay passing the print data or change it after initialization, just as PrintJob currently permits the
    # file to be written to/moved/nonexistent until print is called.
    #
    # Enjoy.

    class Transient < PrintJob

      alias_method :old_init, :initialize

      # Create a print job from a non-empty string. Takes optional printer argument, otherwise uses default.
      # As the tempfile is written to and closed upon initialization, an error will be raised if an empty
      # string is passed as the first argument.
      def initialize(data_string, printer=nil)
        raise "Temporary print job has no data!" if data_string.empty?

        file = Tempfile.new('')
        file.puts(data_string)
        file.close

        old_init(file.path, printer)
      end

    end

  end
end