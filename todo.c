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
 *
 * The todos form a doubly linked list so that we can
 * easily maintain the order.
 */
typedef struct Todo Todo;

struct Todo {
  int idx;
  char label[255];
  bool complete;
  Todo *prev;
  Todo *next;
};

/* this is our app state, we allocate a static amount of memory
   so we don't need to do malloc/free all the time.

   We keep a linked list of used and free todos.
   Each todo also knows its index, so we don't need to traverse
   the list when changing one todo's information.
*/
#define MAX_TODOS 1024
Todo todo_memory[MAX_TODOS]; // memory holding the todos
Todo *todos = NULL;          // list of active todos
Todo *last_todo = NULL;      // the last todo, for adding to end
Todo *free_todos = NULL;     // list of free todos
int num_todos = 0;           // number of active todos

bool valid_idx(int i) { return i >= 0 && i < MAX_TODOS; }

void delete_todo_(int idx) {
  Todo *todo = &todo_memory[idx];
  // remove this from the chain
  if(todo == todos) {
    todos = todo->next;
  }
  if(todo->prev) todo->prev->next = todo->next;
  if(todo->next) todo->next->prev = todo->prev;

  // clear the fields and add this to free_todos
  todo->label[0] = 0;
  todo->complete = false;
  todo->prev = NULL;
  if(free_todos) free_todos->prev = todo;
  todo->next = free_todos;
  free_todos = todo;
}

void delete_todo(int idx) {
  if(!valid_idx(idx)) return;
  delete_todo_(idx);
  rerender();
}

void init_todos() {
  for(int i=MAX_TODOS-1;i>=0;i--) {
    todo_memory[i].idx = i;
    delete_todo_(i);
  }
}

void toggle_todo(int idx) {
  printf("toggling %d\n", idx);
  if(valid_idx(idx)) todo_memory[idx].complete = !todo_memory[idx].complete;
  rerender();
}

void add_todo(char *data) {
  Todo *t = free_todos;
  if(t == NULL) {
    printf("No more free todos!\n");
  } else {
    free_todos = t->next;
    t->next = NULL;
    if(todos == NULL) { // first todo
      todos = t;
    } else {
      last_todo->next = t;
      t->prev = last_todo;
    }
    last_todo = t;

    // copy and allocate our own memory
    size_t len = strlen(data);
    len = len > 255 ? 255 : len;
    memcpy(&t->label[0], data, len);
    t->label[len] = 0;
    t->complete = false;
    t->next = NULL;

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

void todo(Todo *t) {
  tag("li", {
      class(t->complete ? "todo complete" : "todo incomplete");
      attr_i("id", t->idx);
      tag("input", {
          attr_s("type", "checkbox");
          attr_s("onchange", "toggle_todo(event.target.parentNode.id)");
          if(t->complete) attr_b("checked");
        });
      out("%s", t->label);
      tag("button", {
          class("delete");
          attr_s("onclick", "delete_todo(event.target.parentNode.id)");
          out("❌");
        });
    });
}

void form() {
  tag("div", {
      class("form");
      tag("input", {
          attr_s("placeholder", "What needs to be done?");
          attr_s("onchange", "add_todo(event.target.value); event.target.value='';");
        });
    });
}

// Main render function
void todoapp() {
  printf("--------\n");
  tag("div", {
      class("todos");
      tag("ul", {
          Todo *t = todos;
          while(t != NULL) {
            printf("render: idx: %d, label: %s\n", t->idx, t->label);
            if(t->prev) printf("  prev: idx: %d, label: %s\n", t->prev->idx, t->prev->label);
            if(t->next) printf("  next: idx: %d, label: %s\n", t->next->idx, t->next->label);
            todo(t);
            t = t->next;
          }
        });
      form();
    });
}

void rerender() {
  render("app", &todoapp);
}

void init() {
  init_todos();
  rerender();
}
