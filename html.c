#include "html.h"

char _html_memory[HTML_MAX_MEMORY];
char *_html_out;

void tag_start(const char *name) { out("<%s>", name); }

void tag_end(const char *name) { out("</%s>", name); }

static void attr() {
  // back up 1 char, to overwrite the '>' close
  _html_out--;
}
void attr_s(const char *name, const char *value) {
  attr();
  out(" %s=\"%s\">", name, value);
}

void attr_i(const char *name, int value) {
  attr();
  out(" %s=\"%d\">", name, value);
}

void attr_b(const char *name) {
  attr();
  out(" %s>", name);
}

char* get_html() {
  *_html_out = 0;
  _html_out = &_html_memory[0];
  return &_html_memory[0];
}

void class(const char *class) { attr_s("class", class); }
void id(const char *id){ attr_s("id", id); }

int main(int argc, char**argv) {
  html({
      tag("div", {
          id("app");
          class("something");
          tag("span", {
              out("message here: %s", "hello");
            });
          tag("ul", {
              tag("li", { out("list item 1"); })
            });
          tag("input", {
              attr_s("id", "newtodo");
              attr_s("placeholder", "What needs to be done?");
            });
        })
    });
  printf("We got HTML:\n%s", get_html());
  return 0;
}
