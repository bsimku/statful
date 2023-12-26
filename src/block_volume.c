#include "blocks.h"

#include <pulse/operation.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pulse/def.h>
#include <pulse/pulseaudio.h>
#include <pulse/context.h>
#include <pulse/mainloop.h>

#include "alloc.h"
#include "block_common.h"

struct block_volume_data {
    pa_mainloop *ml;
    pa_mainloop_api *ml_api;
    pa_context *ctx;

    enum {
        NONE = 0,
        FAILED,
        READY,
        WAITING
    } state;

    bool sink_changed;
    char sink_name[128];
    unsigned volume;
};

enum pulse_operation {
    OP_SUB_SINK_EVENTS,
    OP_GET_DEFAULT_SINK,
    OP_GET_SINK_VOLUME
};

static bool block_volume_probe() {
    pa_mainloop *ml = pa_mainloop_new();

    if (ml == NULL)
        goto mainloop_fail;

    pa_mainloop_api *ml_api = pa_mainloop_get_api(ml);

    if (ml_api == NULL)
        goto mainloop_fail;

    pa_context *ctx = pa_context_new(ml_api, "test");

    if (ctx == NULL)
        goto context_fail;

    if (pa_context_connect(ctx, NULL, 0, NULL) < 0)
        goto context_fail;

    pa_context_disconnect(ctx);
    pa_context_unref(ctx);
    pa_mainloop_free(ml);

    return true;

context_fail:
    pa_context_unref(ctx);

mainloop_fail:
    pa_mainloop_free(ml);

    return false;
}

static void pa_state_cb(pa_context *ctx, void *userdata) {
    struct block_volume_data *data = userdata;

    const pa_context_state_t state = pa_context_get_state(ctx);

    switch (state) {
        default:
            break;
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            data->state = FAILED;
            break;
        case PA_CONTEXT_READY:
            data->state = READY;
            break;
    }
}

static void pa_server_info_cb(pa_context *c, const pa_server_info *i, void *userdata) {
    struct block_volume_data *data = userdata;

    strncpy(data->sink_name, i->default_sink_name, sizeof(data->sink_name));
}

static void pa_sink_info_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    struct block_volume_data *data = userdata;

    if (i == NULL)
        return;

    const pa_volume_t volume = pa_cvolume_avg(&i->volume);

    data->volume = (unsigned)(((uint64_t)volume * 100 + (uint64_t)PA_VOLUME_NORM / 2) / (uint64_t)PA_VOLUME_NORM);
}

static void pa_context_success_cb(pa_context *c, int success, void *userdata) {
    fprintf(stderr, "success: %d\n", success);
}

static pa_operation *get_operation(struct block_volume_data *data, enum pulse_operation operation) {
    switch (operation) {
        case OP_SUB_SINK_EVENTS:
            return pa_context_subscribe(data->ctx, PA_SUBSCRIPTION_MASK_SERVER | PA_SUBSCRIPTION_MASK_SINK, pa_context_success_cb, data);
        case OP_GET_DEFAULT_SINK:
            return pa_context_get_server_info(data->ctx, pa_server_info_cb, data);
        case OP_GET_SINK_VOLUME:
            return pa_context_get_sink_info_by_name(data->ctx, data->sink_name, pa_sink_info_cb, data);
    }

    return NULL;
}

static bool do_operation(struct block_volume_data *data, enum pulse_operation operation) {
    pa_operation *op;

    while (true) {
        switch (data->state) {
            case NONE:
                pa_mainloop_iterate(data->ml, 1, NULL);
                continue;
            case FAILED:
                return false;
            case READY:
                op = get_operation(data, operation);
                data->state = WAITING;
            case WAITING:
                if (pa_operation_get_state(op) == PA_OPERATION_DONE) {
                    data->state = READY;
                    pa_operation_unref(op);

                    return true;
                }

                break;
        }

        pa_mainloop_iterate(data->ml, 1, NULL);
    }
}

static void pa_context_subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata) {
    struct block_volume_data *data = userdata;

    if ((t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SERVER) {
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_CHANGE) {
            data->sink_changed = true;
        }
    }
}

static bool block_volume_init(void **opaque) {
    struct block_volume_data *data = alloc_zero(sizeof(struct block_volume_data));

    if ((data->ml = pa_mainloop_new()) == NULL)
        return false;

    data->ml_api = pa_mainloop_get_api(data->ml);

    if ((data->ctx = pa_context_new(data->ml_api, "test")) == NULL)
        return false;

    if (pa_context_connect(data->ctx, NULL, 0, NULL) < 0)
        return false;

    pa_context_set_state_callback(data->ctx, pa_state_cb, data);

    if (!do_operation(data, OP_GET_DEFAULT_SINK))
        return false;

    if (!do_operation(data, OP_SUB_SINK_EVENTS))
        return false;

    pa_context_set_subscribe_callback(data->ctx, pa_context_subscribe_cb, data);

    *opaque = data;

    return true;
}

static bool block_volume_update(void *opaque) {
    struct block_volume_data *data = opaque;

    if (data == NULL)
        return false;

    data->volume = -1;

    return true;
}

static bool update_volume(struct block_volume_data *data) {
    if (data->sink_changed) {
        if (!do_operation(data, OP_GET_DEFAULT_SINK))
            return false;

        data->sink_changed = false;
    }

    if (!do_operation(data, OP_GET_SINK_VOLUME))
        return false;

    return true;
}

static int get_var_volume(struct block_volume_data *data) {
    if (data->volume == -1) {
        update_volume(data);
    }

    return data->volume;
}

static char *block_volume_get_var(void *ptr, const char *name, const char *fmt, char *output, size_t size) {
    struct block_volume_data *data = ptr;

    if (data == NULL)
        return NULL;

    BLOCK_PARAM("vol", fmt, get_var_volume(data));

    return output;
}

static bool block_volume_close(void *ptr) {
    struct block_volume_data *data = ptr;

    if (data == NULL)
        return true;

    if (data->ctx) {
        pa_context_disconnect(data->ctx);
        pa_context_unref(data->ctx);
    }

    if (data->ml) {
        pa_mainloop_free(data->ml);
    }

    free(data);

    return true;
}

const struct block block_volume = {
    .name = "volume",
    .probe = block_volume_probe,
    .init = block_volume_init,
    .update = block_volume_update,
    .get_var = block_volume_get_var,
    .close = block_volume_close
};
