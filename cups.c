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

  job_id = cupsPrintFile(target, fname, "Test Print", num_options, options); // Do it.
  return job_id;
}

static VALUE cups_show_dests(VALUE self, VALUE dest_list) {
  int i;
  cups_dest_t *dests, *dest;
  int num_dests = cupsGetDests(&dests); // Size of dest_list array
  dest_list = rb_ary_new();
  
  for (i = num_dests, dest = dests; i > 0; i --, dest ++) {
    VALUE destination = rb_str_new2(dest->name);
    rb_ary_push(dest_list, destination); // Add this testination name to dest_list string
  }
  return dest_list;
}

static VALUE cups_get_default(VALUE self) {
  char *default_printer;
  default_printer = cupsGetDefault();

  if (default_printer != NULL) {
    VALUE def_p = rb_str_new2(default_printer);
    return def_p;
  }
}

VALUE printJobs;

void Init_cups() {
  printJobs = rb_define_class("PrintJob", rb_cObject);
  rb_define_method(printJobs, "initialize", job_init, 2);
  rb_define_method(printJobs, "print", cups_print, 0);
  rb_define_singleton_method(printJobs, "show_destinations", cups_show_dests, 0);
  rb_define_singleton_method(printJobs, "default_printer", cups_get_default, 0);
  id_push = rb_intern("push");
}
