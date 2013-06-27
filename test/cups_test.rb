$LOAD_PATH.unshift File.expand_path( File.dirname(__FILE__) )

require 'rubygems'
require "cups"
require "test/unit"

# The tests which don't make use of mocking go on the operating assumption that you have
# the CUPS command line utilities installed and in your $PATH

class CupsTest < Test::Unit::TestCase

  def setup
    @printer = Cups.show_destinations.select {|p| p =~ /pdf/i}.first
    raise "Can't find a PDF printer to run tests with." unless @printer
  end

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
      Cups::PrintJob.new("/path", @printer)
      Cups::PrintJob.new("/path") if Cups.default_printer
    end
  end

  def test_job_defaults_to_default
    assert_nothing_raised do
      if Cups.default_printer
        pj = Cups::PrintJob.new( "/non/existent/file" )
        assert_equal Cups.default_printer, pj.instance_variable_get(:@printer)
      end
    end
  end

  def test_we_cant_print_to_nonexistent_printers
    assert_raise(ArgumentError) do
      pj = Cups::PrintJob.new("/non/existent/file", "bollocks_printer")
      assert !Cups.show_destinations.include?(pj.instance_variable_get(:@printer))
    end
  end

  def test_we_cant_print_nonexistent_files
    pj = Cups::PrintJob.new("soft_class",@printer)
    assert_raise(RuntimeError) do
          pj.print
    end
    assert_nil pj.job_id
  end

  def test_we_can_pass_args_down_as_options
    options = {:foo => 'bar'}
    pj = Cups::PrintJob.new(sample, @printer, options)
    assert_equal(options, pj.job_options)
  end

  def test_we_can_only_pass_strings_down_as_options
    options = {:foo => 'bar'}
    pj = Cups::PrintJob.new(sample, @printer, options)
    assert_raise(TypeError) { pj.print }
  end

  def test_we_can_omit_options_and_will_set_to_empty
    pj = Cups::PrintJob.new(sample, @printer)
    assert_equal({}, pj.job_options)
  end

  def test_print_job_cancellation
    pj = Cups::PrintJob.new(sample, @printer)
    pj.print
    assert_not_nil pj.job_id
    assert_equal pj.cancel, true
    assert pj.job_id.is_a?(Fixnum)
  end

  def test_all_jobs_raises_with_nonexistent_printers
    assert_raise(RuntimeError) { Cups.all_jobs(nil) }
  end

  def test_all_jobs_returns_hash
    assert Cups.all_jobs(@printer).is_a?(Hash)
  end

  def test_all_jobs_hash_contains_info_hash
    pj = Cups::PrintJob.new(sample, @printer)
    pj.print
    info = Cups.all_jobs(@printer)[pj.job_id]
    assert info.is_a?(Hash)
    assert info.keys.all?{|key| [:title, :format, :submitted_by, :state, :size].include?(key)}
  end

  def test_dest_list_returns_array
    assert Cups.show_destinations.is_a?(Array)
  end

  def test_dest_options_returns_hash_if_real
    assert Cups.options_for(@printer).is_a?(Hash)
  end

  def test_dest_options_raises_exception_if_not_real
    assert_raise(RuntimeError, "The printer or destination doesn't exist!") { Cups.options_for("bollocks_printer") }
  end

  def test_job_failed_boolean
    pj = Cups::PrintJob.new(sample, @printer)
    pj.print
    pj.cancel
    assert !pj.failed?
  end

  def test_job_title
      pj = Cups::PrintJob.new(sample, @printer)
      pj.title = 'a-test-job'
      pj.print
      job = Cups.all_jobs(@printer)[ pj.job_id ]
      assert_equal 'a-test-job', job[:title]
      pj.cancel
  end

  def test_returns_failure_string_on_cancellation
    pj = Cups::PrintJob.new(sample, @printer)
    pj.print

    # assert pj.job_id == 0 # Failed jobs have an ID of zero
    # assert pj.failed?

    # assert pj.error_reason.is_a?(Symbol)
  end

  def test_job_state_string
    pj = Cups::PrintJob.new(sample, @printer)
    assert_nil pj.state # A job can't have a state if it hasn't been assigned a job_id yet
    assert !pj.completed?

    pj.print

    pj.cancel
    assert pj.state == :cancelled
    assert !pj.completed?
  end

  def test_print_job_attributes
    pj = Cups::PrintJob.new(sample, @printer)
    [:printer, :filename, :job_id].each do |attr|
      assert pj.respond_to?(attr)
    end
  end

  private

  def sample
    "#{File.dirname(__FILE__)}/sample.txt"
  end

  def blank_sample
    "#{File.dirname(__FILE__)}/sample_blank.txt"
  end
end
