#include "ms_queue.h"
#include <string.h>
StatusCode ms_queue_create(MsQueueStorage *storage, MsQueueSettings *settings) {
    memcpy(&storage->settings, settings, sizeof(MsQueueSettings));

    storage->handle = xQueueCreateStatic(settings->size, settings->item_size, settings->storage_buf, settings->queue_buf);

    if(storage->handle == NULL) {
        return STATUS_CODE_INVALID_ARGS;
    }

    return STATUS_CODE_OK;
}

StatusCode ms_queue_send(MsQueueStorage *storage, const void *item, uint32_t delay_ticks) {
    BaseType_t ret = xQueueSend(storage->handle, item, delay_ticks);

    if(ret == errQUEUE_FULL) {
        return STATUS_CODE_RESOURCE_EXHAUSTED;
    } else if(ret == pdTRUE) {
        return STATUS_CODE_OK;
    } else {
        // Should always get one of the two status codes above
        return STATUS_CODE_UNREACHABLE;
    }
}

StatusCode ms_queue_receive(MsQueueStorage *storage, void *buf, uint32_t delay_ticks) {
    BaseType_t ret = xQueueReceive(storage->handle, buf, delay_ticks);

    if(ret == pdFALSE) {
        return STATUS_CODE_EMPTY;
    } else if(ret == pdTRUE) {
        return STATUS_CODE_OK;
    } else {
        // Should always get one of the two status codes above
        return STATUS_CODE_UNREACHABLE;
    }
}

StatusCode ms_queue_peek(MsQueueStorage *storage, void *buf, uint32_t delay_ticks) {
    BaseType_t ret = xQueuePeek(storage->handle, buf, delay_ticks);

    if(ret == pdFALSE) {
        return STATUS_CODE_EMPTY;
    } else if(ret == pdTRUE) {
        return STATUS_CODE_OK;
    } else {
        // Should always get one of the two status codes above
        return STATUS_CODE_UNREACHABLE;
    }
}

uint32_t ms_queue_get_size(MsQueueStorage *storage) {
    return storage->settings.size;
}