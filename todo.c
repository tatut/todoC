#include <stdio.h>
#include <stdbool.h>
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>
#include "html.h"

// Need this for interop to turn our C strings in memory to JS strings
EM_JS_DEPS(deps, "$UTF8ToString");

// forward declaration to rerender app
static void rerender();

/* The "model": define a struct for Todo and functions
 * to manipulate them.
 */
typedef struct {
  char *label;
  bool complete;
} Todo;

// this is our app state
int last_todo = 0;
int num_todos = 0;
int capacity = 512; // capacity for todos, if we need more, we must reallocate
Todo *todos;        // memory holding the todos

bool valid_idx(int i) {
  return i >= 0 && i < num_todos;
}

void toggle(int idx) {
  if(valid_idx(idx)) todos[idx].complete = !todos[idx].complete;
}
void add_todo(char *data) {
  if(num_todos > capacity) {
    // FIXME: realloc and grow the array
    exit(1);
  } else {
    // copy and allocate our own memory
    size_t len = strlen(data)+1;
    char *label = malloc(len);
    memcpy(label, data, len);
    int idx = num_todos;
    num_todos ++;
    todos[idx].label = label;
    todos[idx].complete = false;
    printf("Added todo: %s\n", label);
    printf("len(incl pad): %ld\n", len);
    rerender();
  }
}



/**
 * Utility that takes a DOM element id and a function pointer.
 * Renders HTML by calling the function and sets the result
 * as the innerHTML of the DOM element.
 *
 * NOTE: we could use morphdom or some library here, but this suffices
 * for our purposes.
 */
static void render(const char* to_element_id, void (*render_fn)()) {
  html({ render_fn(); });
  char *html = get_html();
  EM_ASM({
      var id = UTF8ToString($0);
      var content = UTF8ToString($1);
      document.getElementById(id).innerHTML = content;
    }, to_element_id, html);
}

void todo(Todo t) {
  tag("li", {
      class(t.complete ? "todo complete" : "todo incomplete");
      out("%s", t.label);
    });
}

// Main render function
void todoapp() {
  tag("div", {
      class("todos");
      tag("ul", {
          for(int i=0; i < num_todos; i++) {
            todo(todos[i]);
          }
        });
    });
}

static void rerender() {
  render("app", &todoapp);
}

void init() {
  todos = malloc(capacity * sizeof(Todo));
  rerender();
}
