#include <ruby_cups.h>

cups_dest_t *dests, *dest;
VALUE rubyCups, printJobs;

// Need to abstract this out of cups.c
VALUE ipp_state_to_symbol(int state)
{
  VALUE jstate;

  switch (state) {
    case IPP_JOB_PENDING :
      jstate = ID2SYM(rb_intern("pending"));
      break;
    case IPP_JOB_HELD :
      jstate = ID2SYM(rb_intern("held"));
      break;
    case IPP_JOB_PROCESSING :
      jstate = ID2SYM(rb_intern("processing"));
      break;
    case IPP_JOB_STOPPED :
      jstate = ID2SYM(rb_intern("stopped"));
      break;
    case IPP_JOB_CANCELED :
      jstate = ID2SYM(rb_intern("cancelled"));
      break;
    case IPP_JOB_ABORTED :
      jstate = ID2SYM(rb_intern("aborted"));
      break;
    case IPP_JOB_COMPLETED :
      jstate = ID2SYM(rb_intern("completed"));
      break;
    default:
      jstate = ID2SYM(rb_intern("unknown"));
  }
  return jstate;
}

int printer_exists(VALUE printer){
  // First call Cups#show_destinations
  VALUE dest_list = rb_funcall(rubyCups, rb_intern("show_destinations"), 0);
  // Then check the printer arg is included in the returned array...
  return rb_ary_includes(dest_list, printer) ? 1 : 0;
}

/*
* call-seq:
*   PrintJob.new(filename, printer=nil)
*
* Initializes a new PrintJob object. If no target printer/class is specified, the default is chosen.
* Note the specified file does not have to exist until print is called.
*/
static VALUE job_init(int argc, VALUE* argv, VALUE self)
{
  VALUE filename, printer, job_options;

  rb_scan_args(argc, argv, "12", &filename, &printer, &job_options);

  rb_iv_set(self, "@filename", filename);
  rb_iv_set(self, "@url_path", rb_str_new2(cupsServer()));

  if (NIL_P(job_options)) {
    rb_iv_set(self, "@job_options", rb_hash_new());
  } else {
    rb_iv_set(self, "@job_options", job_options);
  }

  if (NIL_P(printer)) {

    // Fall back to default printer
    VALUE def_p = rb_funcall(rubyCups, rb_intern("default_printer"), 0);

    if (def_p == Qfalse) {
			rb_raise(rb_eRuntimeError, "There is no default printer!");
		} else {
			rb_iv_set(self, "@printer", def_p);
		}

  } else {
    if (printer_exists(printer)) {
      rb_iv_set(self, "@printer", printer);
    } else {
      rb_raise(rb_eRuntimeError, "The printer or destination doesn't exist!");
    }
  }
  return self;
}

/*
 * Note: rb_hash_keys is defined in 1.8.6, but not in 1.8.7 ubuntu shared lib
 * This is so that I can get a list of keys to convert to options
 */
static int
cups_keys_i(key, value, ary)
  VALUE key, value, ary;
{
  if (key == Qundef) return ST_CONTINUE;
  rb_ary_push(ary, key);
  return ST_CONTINUE;
}

/*
* call-seq:
*   print_job.print -> Fixnum
*
* Submit a print job to the selected printer or class. Returns true on success.
*/
static VALUE cups_print(VALUE self)
{
  int job_id;
  VALUE file = rb_iv_get(self, "@filename");
  VALUE rname = rb_iv_get(self, "@title");
  VALUE printer = rb_iv_get(self, "@printer");
  VALUE url_path = rb_iv_get(self, "@url_path");

  char *fname = RSTRING_PTR(file); // Filename
  char *title = T_STRING == TYPE(rname) ? RSTRING_PTR(rname) : "rCups";
  char *target = RSTRING_PTR(printer); // Target printer string
  const char *url = RSTRING_PTR(url_path); // Server URL address
  int port = 631; // Default CUPS port

  VALUE job_options = rb_iv_get(self, "@job_options");

  // Create an array of the keys from the job_options hash
  VALUE job_options_keys = rb_ary_new();
  rb_hash_foreach(job_options, cups_keys_i, job_options_keys);

  VALUE iter;
  int num_options = 0;
  cups_option_t *options = NULL;

  // foreach option in the job options array
  while (!  NIL_P(iter = rb_ary_pop(job_options_keys))) {

    VALUE value = rb_hash_aref(job_options, iter);

    // assert the key and value are strings
    if (NIL_P(rb_check_string_type(iter)) || NIL_P(rb_check_string_type(value))) {
      cupsFreeOptions(num_options, options);
      rb_raise(rb_eTypeError, "job options is not string => string hash");
      return Qfalse;
    }

    // convert to char pointers and add to cups optoins
    char * iter_str  = rb_string_value_ptr(&iter);
    char * value_str = rb_string_value_ptr(&value);
    cupsAddOption(iter_str, value_str, num_options++, &options);
  }

  if(NIL_P(url)) {
    url = cupsServer();
  }

  int encryption = (http_encryption_t)cupsEncryption();
  http_t *http = httpConnect2(url, port, NULL, AF_UNSPEC, (http_encryption_t) encryption, 1, 30000, NULL);

  job_id = cupsPrintFile2(http, target, fname, title, num_options, options); // Do it. "rCups" should be the filename/path
  //
  // cupsFreeOptions(num_options, options);

  rb_iv_set(self, "@job_id", INT2NUM(job_id));
  return Qtrue;
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

  cupsFreeDests(num_dests, dests);
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
  } else {
    return Qnil;
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

    jstate = ipp_state_to_symbol(job_state);
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
  }

  num_jobs = cupsGetJobs(&jobs, printer_arg, 1, -1); // Get jobs

  // find our job
  for (i = 0; i < num_jobs; i ++) {
    if (jobs[i].id == NUM2INT(job_id)) {
      job_state = jobs[i].state;
      break;
    }
  }

  // Free job array
  cupsFreeJobs(num_jobs, jobs);

  if (job_state == IPP_JOB_COMPLETED) {
    return Qtrue;
  } else {
    return Qfalse;
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
  // Don't have to lift a finger unless the printer exists.
  if (!printer_exists(printer)){
    rb_raise(rb_eRuntimeError, "The printer or destination doesn't exist!");
  }

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
    jstate = ipp_state_to_symbol(jobs[i].state);

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
*   Cups.cancel_print(cups_id, printer_name) -> true or false
*
* Cancel the print job. Returns true if successful, false otherwise.
*/
static VALUE cups_cancel_print(int argc, VALUE* argv, VALUE self)
{
  VALUE printer, job_id;
  rb_scan_args(argc, argv, "20", &job_id, &printer);

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
 *  call-seq:
 *    Cups.device_uri_for(printer_name) -> String
 *
 *  Return uri for requested printer.
 */
static VALUE cups_get_device_uri(VALUE self, VALUE printer)
{
   if (!printer_exists(printer))
   {
     rb_raise(rb_eRuntimeError, "The printer or destination doesn't exist!");
   }

   VALUE options_list;
   http_t *http;
   ipp_t *request;
   ipp_t *response;
   ipp_attribute_t *attr;
   char uri[1024];
   char *location;
   char *name = RSTRING_PTR(printer);

   request = ippNewRequest(IPP_GET_PRINTER_ATTRIBUTES);
   httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL, "localhost", 0, "/printers/%s", name);
   ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);

   if ((response = cupsDoRequest(http, request, "/")) != NULL)
   {
     if((attr = ippFindAttribute(response, "device-uri", IPP_TAG_URI)) != NULL)
     {
       return rb_str_new2(attr->values[0].string.text);
     }
     ippDelete(response);
   }
   return Qtrue;
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
  // Don't have to lift a finger unless the printer exists.
  if (!printer_exists(printer)){
    rb_raise(rb_eRuntimeError, "The printer or destination doesn't exist!");
  }

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

/*
*/

void Init_cups() {
  rubyCups = rb_define_module("Cups");
  printJobs = rb_define_class_under(rubyCups, "PrintJob", rb_cObject);

  // Cups::PrintJob Attributes
  rb_define_attr(printJobs, "printer", 1, 0);
  rb_define_attr(printJobs, "filename", 1, 0);
  rb_define_attr(printJobs, "url_path", 1, 0);
  rb_define_attr(printJobs, "job_id", 1, 0);
  rb_define_attr(printJobs, "job_options", 1, 0);
  rb_define_attr(printJobs, "title", 1, 1);

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
  rb_define_singleton_method(rubyCups, "cancel_print", cups_cancel_print, -1);
  rb_define_singleton_method(rubyCups, "options_for", cups_get_options, 1);
  rb_define_singleton_method(rubyCups, "device_uri_for", cups_get_device_uri, 1);
  rb_define_singleton_method(rubyCups, "get_connection_for", cups_get_device_uri, 1);
}
