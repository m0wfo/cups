require "cups"
require "test/unit"

class CupsTest < Test::Unit::TestCase
  def test_same_printers_returned
    lplist = `lpstat -a`.split("\n").map { |pr| pr.split(' ').first }
    cups_destinations = Cups.show_destinations
    assert cups_destinations.is_a?(Array)
    assert_equal cups_destinations, lplist
  end
  
  def test_can_instantiate_print_job_object_with_correct_args
    assert_raises(ArgumentError) do
      Cups::PrintJob.new
    end
    
    assert_nothing_raised do
      Cups::PrintJob.new("test_printer", "/path")
    end
  end
  
  def test_we_cant_print_to_nonexistant_printers
    true
  end
end
