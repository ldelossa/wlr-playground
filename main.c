#include <error.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-util.h>

typedef struct thread_dispatcher_args {
  struct wl_display *display;
} thread_dispatcher_args;

void *thread_dispatcher(void *arg) {
  thread_dispatcher_args *td_args = (thread_dispatcher_args *)arg;
  int wl_display_fd = wl_display_get_fd(td_args->display);
  printf("Got wl_display_fd: %d\n", wl_display_fd);

  int ep_fd;
  if ((ep_fd = epoll_create1(0)) == -1) {
    perror("epoll_create1");
    exit(EXIT_FAILURE);
  }

  struct epoll_event ev = {.events = EPOLLIN, .data.fd = wl_display_fd};

  if (epoll_ctl(ep_fd, EPOLL_CTL_ADD, wl_display_fd, &ev) == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }

  struct epoll_event events[1];
  while (epoll_wait(ep_fd, events, 1, -1) != -1) {
    if (wl_display_dispatch(td_args->display) == -1) {
      perror("wl_display_dispatch");
      exit(EXIT_FAILURE);
    };
    wl_display_flush(td_args->display);
  }

  return NULL;
}

void global(void *data, struct wl_registry *wl_registry, uint32_t name,
            const char *interface, uint32_t version) {
  printf("Received Global: %d %s\n", name, interface);
}

int main(int argc, char *argv[]) {

  struct wl_display *display = wl_display_connect(NULL);

  if (!display)
    return -1;

  printf("Successfully connected to wayland display!\n");

  // launch the event loop in a separate thread.
  pthread_t thread;
  thread_dispatcher_args args = {.display = display};
  pthread_create(&thread, NULL, thread_dispatcher, &args);

  struct wl_registry *registry = wl_display_get_registry(display);
  if (!display)
    goto error;

  struct wl_registry_listener reg_lsnr = {.global = global};

  wl_registry_add_listener(registry, &reg_lsnr, NULL);

  wl_display_flush(display);

  pthread_join(thread, NULL);

  return 0;

error:
  wl_display_disconnect(display);
  return -1;
}
