/*
 * wl-launch: Wayland-native app launcher with startup notification cursor.
 * Sets DESKTOP_STARTUP_ID (gtk-shell) and XDG_ACTIVATION_TOKEN for the child.
 *
 * Usage: wl-launch <command> [args...]
 *    or: wl-launch --desktop <name>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <wayland-client.h>
#include "xdg-activation-v1-client.h"

static struct wl_display *display;
static struct wl_seat *seat;
static struct xdg_activation_v1 *activation;
static char *token_str;
static int token_received;

static void registry_global(void *data, struct wl_registry *registry,
                            uint32_t name, const char *interface, uint32_t version) {
    if (strcmp(interface, "wl_seat") == 0)
        seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    else if (strcmp(interface, "xdg_activation_v1") == 0)
        activation = wl_registry_bind(registry, name, &xdg_activation_v1_interface, 1);
}

static void registry_global_remove(void *data, struct wl_registry *registry, uint32_t name) {}

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

static void token_done(void *data, struct xdg_activation_token_v1 *token, const char *tok) {
    token_str = strdup(tok);
    token_received = 1;
}

static const struct xdg_activation_token_v1_listener token_listener = {
    .done = token_done,
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: wl-launch <command> [args...]\n");
        fprintf(stderr, "       wl-launch --desktop <name>\n");
        return 1;
    }

    int desktop_mode = 0;
    int cmd_start = 1;
    if (strcmp(argv[1], "--desktop") == 0) {
        if (argc < 3) return 1;
        desktop_mode = 1;
        cmd_start = 2;
    }

    /* Generate a DESKTOP_STARTUP_ID for gtk-shell cursor */
    char startup_id[128];
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    snprintf(startup_id, sizeof(startup_id), "wl-launch-%d-%ld_TIME%ld",
             getpid(), ts.tv_sec, ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    setenv("DESKTOP_STARTUP_ID", startup_id, 1);

    /* Try to get xdg-activation token */
    display = wl_display_connect(NULL);
    if (display) {
        struct wl_registry *registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &registry_listener, NULL);
        wl_display_roundtrip(display);

        if (activation) {
            struct xdg_activation_token_v1 *token =
                xdg_activation_v1_get_activation_token(activation);
            xdg_activation_token_v1_add_listener(token, &token_listener, NULL);
            xdg_activation_token_v1_commit(token);
            wl_display_flush(display);

            token_received = 0;
            for (int i = 0; i < 50 && !token_received; i++)
                wl_display_dispatch(display);

            xdg_activation_token_v1_destroy(token);
            xdg_activation_v1_destroy(activation);
        }
        if (seat) wl_seat_destroy(seat);
        wl_display_disconnect(display);

        if (token_str)
            setenv("XDG_ACTIVATION_TOKEN", token_str, 1);
    }

    /* Launch */
    if (desktop_mode)
        execlp("gtk-launch", "gtk-launch", argv[cmd_start], NULL);
    else
        execvp(argv[cmd_start], &argv[cmd_start]);

    perror("wl-launch: exec");
    return 1;
}
