require "cups"
require "test/unit"
require "flexmock/test_unit"

# The tests which don't make use of mocking go on the operating assumption that you have
# the CUPS command line utilities installed and in your $PATH

class CupsTest < Test::Unit::TestCase
  def test_same_printers_returned
    lplist = `lpstat -a`.split("\n").map { |pr| pr.split(' ').first }
    cups_destinations = Cups.show_destinations
    assert cups_destinations.is_a?(Array)
    assert_equal cups_destinations, lplist
  end
  
  def test_can_instantiate_print_job_object_with_correct_args
    assert_raise(ArgumentError) do
      Cups::PrintJob.new
    end
    
    assert_nothing_raised do
      Cups::PrintJob.new("/path", "PDF_Printer")
      Cups::PrintJob.new("/path")
    end
  end
  
  def test_job_defaults_to_default
    assert_nothing_raised do
      pj = Cups::PrintJob.new("/non/existent/file")

      assert_equal pj.instance_variable_get(:@printer), Cups.default_printer
    end
  end
  
  def test_we_cant_print_to_nonexistent_printers
    assert_raise(RuntimeError) do
      pj = Cups::PrintJob.new("/non/existent/file", "bollocks_printer")
      
      assert !Cups.show_destinations.include?(pj.instance_variable_get(:@printer))
    end
  end
  
  def test_we_cant_print_nonexistent_files
    pj = Cups::PrintJob.new("soft_class")
    
    assert_raise(RuntimeError) do
      pj.print
    end
  
    assert_nil pj.job_id
  end
  
  def test_print_job_cancellation
    pj = Cups::PrintJob.new("#{Dir.pwd}/sample.txt", "soft_class")
    pj.print
    assert_not_nil pj.job_id
    assert_equal pj.cancel, true
    assert pj.job_id.is_a?(Fixnum)
  end
end
