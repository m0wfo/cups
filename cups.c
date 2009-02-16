#include <cups/cups.h>
#include <ruby/ruby.h>

#ifndef MAXOPTS
  #define MAXOPTS 100
#endif

static int id_push;
static int num_options;
static cups_option_t *options;

static VALUE job_init(VALUE self, VALUE filename, VALUE printer) {
  rb_iv_set(self, "@filename", filename);
  rb_iv_set(self, "@printer", printer);
  return self;
}

static VALUE cups_print(VALUE self, VALUE file, VALUE printer) {
  int job_id;
  file = rb_iv_get(self, "@filename");
  printer = rb_iv_get(self, "@printer");
  
  char *fname = RSTRING_PTR(file); // Filename
  char *target = RSTRING_PTR(printer); // Target printer string

  job_id = cupsPrintFile(target, fname,
                          "Test Print", num_options, options);
  return job_id;
}

VALUE printJobs;

void Init_cups() {
  printJobs = rb_define_class("PrintJob", rb_cObject);
  rb_define_method(printJobs, "initialize", job_init, 2);
  rb_define_method(printJobs, "print", cups_print, 0);
  id_push = rb_intern("push");
}
