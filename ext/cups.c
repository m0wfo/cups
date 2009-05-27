#include <ruby_cups.h>

static int num_options;
static cups_option_t *options;
cups_dest_t *dests, *dest;
VALUE rubyCups, printJobs;


/*
* call-seq:
*   PrintJob.new(filename, printer=nil)
*
* Initializes a new PrintJob object. If no target printer/class is specified, the default is chosen.
* Note the specified file does not have to exist until print is called.
*/
static VALUE job_init(int argc, VALUE* argv, VALUE self)
{
  VALUE filename, printer;
  
  rb_scan_args(argc, argv, "11", &filename, &printer);
  
  rb_iv_set(self, "@filename", filename);
  
  if (NIL_P(printer)) {

    // Fall back to default printer
    VALUE def_p = rb_funcall(rubyCups, rb_intern("default_printer"), 0);
    rb_iv_set(self, "@printer", def_p);
    
  } else {
    // First call Cups#show_destinations
    VALUE dest_list = rb_funcall(rubyCups, rb_intern("show_destinations"), 0);
    // Then check the printer arg is included in the returned array...
    if (rb_ary_includes(dest_list, printer)) {
      rb_iv_set(self, "@printer", printer);
    } else {
      rb_raise(rb_eRuntimeError, "The printer or destination doesn't exist!");
    }
  }
  return self;
}

/*
* call-seq:
*   print_job.print -> Fixnum
*
* Submit a print job to the selected printer or class. Returns true on success.
*/
static VALUE cups_print(VALUE self, VALUE file, VALUE printer)
{
  int job_id;
  file = rb_iv_get(self, "@filename");
  printer = rb_iv_get(self, "@printer");
  
  char *fname = RSTRING_PTR(file); // Filename
  char *target = RSTRING_PTR(printer); // Target printer string
  
  FILE *fp = fopen(fname,"r");
  // Check @filename actually exists...
  if( fp ) {
    fclose(fp);
    job_id = cupsPrintFile(target, fname, "rCUPS", num_options, options); // Do it.
    rb_iv_set(self, "@job_id", INT2NUM(job_id));
    return Qtrue;
  } else {
  // and if it doesn't...
    rb_raise(rb_eRuntimeError, "Couldn't find file");
    return self;
  }
}

/*
* call-seq:
*   Cups.show_destinations -> Array
*
* Show all destinations on the default server
*/
static VALUE cups_show_dests(VALUE self)
{
  VALUE dest_list;
  int i;
  int num_dests = cupsGetDests(&dests); // Size of dest_list array
  dest_list = rb_ary_new2(num_dests);
  
  for (i = num_dests, dest = dests; i > 0; i --, dest ++) {
    VALUE destination = rb_str_new2(dest->name);
    rb_ary_push(dest_list, destination); // Add this testination name to dest_list string
  }
  return dest_list;
}

/*
* call-seq:
*   Cups.default_printer -> String or nil
*
* Get default printer or class. Returns a string or false if there is no default
*/
static VALUE cups_get_default(VALUE self)
{
  const char *default_printer;
  default_printer = cupsGetDefault();

  if (default_printer != NULL) {
    VALUE def_p = rb_str_new2(default_printer);
    return def_p;
  }
}

/*
* call-seq:
*   print_job.cancel -> true or false
*
* Cancel the current job. Returns true if successful, false otherwise.
*/
static VALUE cups_cancel(VALUE self)
{
  VALUE printer, job_id;
  printer = rb_iv_get(self, "@printer");
  job_id = rb_iv_get(self, "@job_id");
  
  if (NIL_P(job_id)) {
    return Qfalse; // If @job_id is nil
  } else { // Otherwise attempt to cancel
    int job = NUM2INT(job_id);
    char *target = RSTRING_PTR(printer); // Target printer string
    int cancellation;
    cancellation = cupsCancelJob(target, job);
    return Qtrue;
  }
}

/*
* call-seq:
*   print_job.failed? -> true or false
*
* Did this job fail?
*/
static VALUE cups_job_failed(VALUE self)
{
  VALUE job_id = rb_iv_get(self, "@job_id");

 if (NIL_P(job_id) || !NUM2INT(job_id) == 0) {
   return Qfalse;
 } else {
   return Qtrue;
 }
}

/*
* call-seq:
*   print_job.error_reason -> String
*
* Get the last human-readable error string
*/
static VALUE cups_get_error_reason(VALUE self)
{
  VALUE job_id = rb_iv_get(self, "@job_id");
  
  if (NIL_P(job_id) || !NUM2INT(job_id) == 0) {
    return Qnil;
  } else {
    VALUE error_exp = rb_str_new2(cupsLastErrorString());
    return error_exp;
  }
}

/*
* call-seq:
*   print_job.error_code -> Fixnum
*
* Get the last IPP error code.
*/
static VALUE cups_get_error_code(VALUE self)
{
  VALUE job_id = rb_iv_get(self, "@job_id");
  
  if (NIL_P(job_id) || !NUM2INT(job_id) == 0) {
    return Qnil;
  } else {
    VALUE ipp_error_code = INT2NUM(cupsLastError());
    return ipp_error_code;
  }
}

/*
* call-seq:
*   print_job.state -> String
*
* Get human-readable state of current job.
*/
static VALUE cups_get_job_state(VALUE self)
{
  VALUE job_id = rb_iv_get(self, "@job_id");
  VALUE printer = rb_iv_get(self, "@printer");
  VALUE jstate;
  
  int num_jobs;
  cups_job_t *jobs;
  ipp_jstate_t job_state = IPP_JOB_PENDING;
  int i;
  char *printer_arg = RSTRING_PTR(printer);
 
  if (NIL_P(job_id)) {
    return Qnil;
  } else {
    num_jobs = cupsGetJobs(&jobs, printer_arg, 1, -1); // Get jobs

    for (i = 0; i < num_jobs; i ++) {
      if (jobs[i].id == NUM2INT(job_id)) {
        job_state = jobs[i].state;
        break;
      }
    }

     // Free job array
     cupsFreeJobs(num_jobs, jobs);
 
    switch (job_state) {
      case IPP_JOB_PENDING :
        jstate = rb_str_new2("Pending...");
        break;
      case IPP_JOB_HELD :
        jstate = rb_str_new2("Held");
        break;
      case IPP_JOB_PROCESSING :
        jstate = rb_str_new2("Processing...");
        break;
      case IPP_JOB_STOPPED :
        jstate = rb_str_new2("Stopped");
        break;
      case IPP_JOB_CANCELED :
        jstate = rb_str_new2("Cancelled");
        break;
      case IPP_JOB_ABORTED :
        jstate = rb_str_new2("Aborted");
        break;
      case IPP_JOB_COMPLETED :
		jstate = rb_str_new2("Completed");
        break;
      default:
		jstate = rb_str_new2("Unknown Job Code...");
    }

    return jstate;
  }
}

/*
* call-seq:
*   print_job.completed? -> true or false
*
* Has the job completed?
*/
static VALUE cups_job_completed(VALUE self)
{
  VALUE job_id = rb_iv_get(self, "@job_id");
  VALUE printer = rb_iv_get(self, "@printer");
  VALUE jstate;

  int num_jobs;
  cups_job_t *jobs;
  ipp_jstate_t job_state = IPP_JOB_PENDING;
  int i;
  char *printer_arg = RSTRING_PTR(printer);

  if (NIL_P(job_id)) {
    return Qfalse;
  } else {
    num_jobs = cupsGetJobs(&jobs, printer_arg, 1, -1); // Get jobs
    // job_state = IPP_JOB_COMPLETED;

    for (i = 0; i < num_jobs; i ++) {
      if (jobs[i].id == NUM2INT(job_id)) {
        job_state = jobs[i].state;
        break;
      }
      
      // Free job array
      cupsFreeJobs(num_jobs, jobs);
      
      if (job_state == IPP_JOB_COMPLETED) {
        return Qtrue;
      } else {
        return Qfalse;
      }
      
    }
  }  
}

/*
* call-seq:
*   Cups.all_jobs(printer) -> Hash
*
* Get all jobs from default CUPS server. Takes a single printer/class string argument.
* Returned hash keys are CUPS job ids, and the values are hashes of job info
* with keys:
*
* [:title, :submitted_by, :size, :format, :state]
*/
static VALUE cups_get_jobs(VALUE self, VALUE printer)
{
  VALUE job_list, job_info_hash, jid, jtitle, juser, jsize, jformat, jstate;
  int job_id;
  int num_jobs;
  cups_job_t *jobs;
  ipp_jstate_t state;
  int i;
  char *printer_arg = RSTRING_PTR(printer);
  
  num_jobs = cupsGetJobs(&jobs, printer_arg, 1, -1); // Get jobs
  job_list = rb_hash_new();
  
  for (i = 0; i < num_jobs; i ++) { // Construct a hash of individual job info
    job_info_hash = rb_hash_new();
    jid = INT2NUM(jobs[i].id);
    jtitle = rb_str_new2(jobs[i].title);
    juser = rb_str_new2(jobs[i].user);
    jsize = INT2NUM(jobs[i].size);
    jformat = rb_str_new2(jobs[i].format);
    
    // Let's elaborate on that job state...
    switch (jobs[i].state) {
      case IPP_JOB_PENDING :
        jstate = rb_str_new2("Pending...");
        break;
      case IPP_JOB_HELD :
        jstate = rb_str_new2("Held");
        break;
      case IPP_JOB_PROCESSING :
        jstate = rb_str_new2("Processing...");
        break;
      case IPP_JOB_STOPPED :
        jstate = rb_str_new2("Stopped");
        break;
      case IPP_JOB_CANCELED :
        jstate = rb_str_new2("Cancelled");
        break;
      case IPP_JOB_ABORTED :
        jstate = rb_str_new2("Aborted");
        break;
      case IPP_JOB_COMPLETED :
        jstate = rb_str_new2("Completed");
    }
    
    rb_hash_aset(job_info_hash, ID2SYM(rb_intern("title")), jtitle);
    rb_hash_aset(job_info_hash, ID2SYM(rb_intern("submitted_by")), juser);
    rb_hash_aset(job_info_hash, ID2SYM(rb_intern("size")), jsize);
    rb_hash_aset(job_info_hash, ID2SYM(rb_intern("format")), jformat);
    rb_hash_aset(job_info_hash, ID2SYM(rb_intern("state")), jstate);
    
    rb_hash_aset(job_list, jid, job_info_hash); // And push it all into job_list hash
  }

  // Free job array
  cupsFreeJobs(num_jobs, jobs);

  return job_list;
}

/*
 * call-seq:
 *   Cups.options_for(name) -> Hash or nil
 *
 * Get all options from CUPS server with name. Returns a hash with key/value pairs
 * based on server options, or nil if no server with name.
 */
static VALUE cups_get_options(VALUE self, VALUE printer)
{
  VALUE options_list;
  int i;
  char *printer_arg = RSTRING_PTR(printer);

  options_list = rb_hash_new();

  cups_dest_t *dests;
  int num_dests = cupsGetDests(&dests);
  cups_dest_t *dest = cupsGetDest(printer_arg, NULL, num_dests, dests);

  if (dest == NULL) {
    cupsFreeDests(num_dests, dests);
    return Qnil;
  } else {
    for(i =0; i< dest->num_options; i++) {
      rb_hash_aset(options_list, rb_str_new2(dest->options[i].name), rb_str_new2(dest->options[i].value));
    }

    cupsFreeDests(num_dests, dests);
    return options_list;
  }

}

// TODO
// int ipp_state_to_string(int state)
// {
//   // char *jstate;
//   switch (state) {
//     case IPP_JOB_PENDING :
//       // jstate = rb_str_new2("Pending...");
//       // char jstate[] = "Pending...";
//       break;
//     case IPP_JOB_HELD :
//       // jstate = rb_str_new2("Held");
//       // char jstate[] = "Held";
//       break;
//     case IPP_JOB_PROCESSING :
//       // jstate = rb_str_new2("Processing...");
//       // char jstate[] = "Processing...";
//       break;
//     case IPP_JOB_STOPPED :
//       // jstate = rb_str_new2("Stopped");
//       // char jstate[] = "Stopped";
//       break;
//     case IPP_JOB_CANCELED :
//       // jstate = rb_str_new2("Cancelled");
//       // char jstate[] = "Cancelled";
//       break;
//     case IPP_JOB_ABORTED :
//       // jstate = rb_str_new2("Aborted");
//       // char jstate[] = "Aborted";
//       break;
//     case IPP_JOB_COMPLETED :
//       // jstate = rb_str_new2("Completed");
//       break;
//   }
//   return 0;
// }

/*
*/

void Init_cups() {
  rubyCups = rb_define_module("Cups");
  printJobs = rb_define_class_under(rubyCups, "PrintJob", rb_cObject);

  // Cups::PrintJob Attributes
  rb_define_attr(printJobs, "printer", 1, 0);
  rb_define_attr(printJobs, "filename", 1, 0);
  rb_define_attr(printJobs, "job_id", 1, 0);

  // Cups::PrintJob Methods
  rb_define_method(printJobs, "initialize", job_init, -1);
  rb_define_method(printJobs, "print", cups_print, 0);
  rb_define_method(printJobs, "cancel", cups_cancel, 0);
  rb_define_method(printJobs, "state", cups_get_job_state, 0);
  rb_define_method(printJobs, "completed?", cups_job_completed, 0);
  rb_define_method(printJobs, "failed?", cups_job_failed, 0);
  rb_define_method(printJobs, "error_reason", cups_get_error_reason, 0);
  rb_define_method(printJobs, "error_code", cups_get_error_code, 0);

  // Cups Module Methods
  rb_define_singleton_method(rubyCups, "show_destinations", cups_show_dests, 0);
  rb_define_singleton_method(rubyCups, "default_printer", cups_get_default, 0);
  rb_define_singleton_method(rubyCups, "all_jobs", cups_get_jobs, 1);
  rb_define_singleton_method(rubyCups, "options_for", cups_get_options, 1);
}
