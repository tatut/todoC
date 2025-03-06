#ifndef html_h
#define html_h

#include <stdio.h>

// I have it on good authority that 640k ought to be enough for anyone
#define HTML_MAX_MEMORY 640 * 1024

extern char _html_memory[HTML_MAX_MEMORY];
extern char *_html_out;

// Macro to append formatted message to HTML memory at current pos
#define out(args...)                                                      \
  _html_out +=                                                                 \
      snprintf(_html_out, (&_html_memory[HTML_MAX_MEMORY] - _html_out - 1), args)

void tag_start(const char *name);
void tag_end(const char *name);
void attr_s(const char *name, const char *value);
void attr_i(const char *name, int value);
void attr_b(const char *name);

void class(const char *class);
void id(const char *id);

// get current built HTML as 0 terminated string
char *get_html();



#define html(body)                                                      \
  {                                                                     \
    _html_out = &_html_memory[0];                                       \
    body                                                                \
      }

#define tag(name, body)                                                        \
  {                                                                            \
    tag_start(name);                                                           \
    body tag_end(name);                                                        \
  }

#endif
