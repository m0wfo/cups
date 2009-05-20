require "cups"
require "test/unit"

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

      assert_equal Cups.default_printer, pj.instance_variable_get(:@printer)
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
    pj = Cups::PrintJob.new(sample, "soft_class")
    pj.print
    assert_not_nil pj.job_id
    assert_equal pj.cancel, true
    assert pj.job_id.is_a?(Fixnum)
  end
  
  def test_all_jobs_returns_hash
    assert Cups.all_jobs(Cups.default_printer).is_a?(Hash)
  end
  
  def test_dest_list_returns_array
    assert Cups.show_destinations.is_a?(Array)
  end
  
  def test_job_failed_boolean
    pj = Cups::PrintJob.new(sample, "soft_class")
    pj.print
    pj.cancel
    assert !pj.failed?
  end
  
  def test_returns_failure_string_on_cancellation
    pj = Cups::PrintJob.new(blank_sample, "PDF_Printer")
    pj.print
    
    assert pj.job_id == 0 # Failed jobs have an ID of zero
    assert pj.failed?
    
    assert pj.error_reason.is_a?(String)
  end
  
  def test_job_state_string
    pj = Cups::PrintJob.new(sample, "soft_class")
    assert_nil pj.state # A job can't have a state if it hasn't been assigned a job_id yet
    assert !pj.completed?

    pj.print
    assert pj.state == "Pending..."

    pj.cancel
    assert !pj.completed?
  end
  
  def test_print_job_attributes
    pj = Cups::PrintJob.new(sample)
    [:printer, :filename, :job_id].each do |attr|
      assert pj.respond_to?(attr)
    end
  end

  private
  
  def sample
    "#{Dir.pwd}/sample.txt"
  end
  
  def blank_sample
    "#{Dir.pwd}/sample_blank.txt"
  end
end
