#include "connmgr.h"

#include "config.h"
#include "lib/dplist.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <time.h>
#include <unistd.h>

void socket_free(void** socket) {
    tcp_close((tcpsock_t**) socket);
}

void connmgr_listen(int port_number, sbuffer_t* buffer) {

#if DEBUG
    const int fd =
        open("sensor_data_recv", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    assert(fd > 0);
#endif

    dplist_t* list = dpl_create(NULL, socket_free, NULL);

    {
        tcpsock_t* connection_socket = NULL;
        if (tcp_passive_open(&connection_socket, port_number) != TCP_NO_ERROR)
            exit(EXIT_FAILURE);
        dpl_insert_at_index(list, connection_socket, 0, false);
    }

    bool active = true;
    struct pollfd* fds = NULL;
    while (active) {
        fds = realloc(fds, dpl_size(list) * sizeof(*fds));

        for (iterator_t it = it_begin(list); !it_is_null(&it); it_incr(&it)) {
            tcpsock_t* socket = it_get_element(&it);
            fds[it.index] = (struct pollfd){
                .fd = socket->sd,
                .events = POLLIN,
            };
        }

        int n = poll(fds, dpl_size(list), TIMEOUT * 1000);
        assert(n != -1);

        if (n == 0) {
            // quit the connmgr (TIMEOUT was reached)
            printf("No sensor data received after " TO_STRING(TIMEOUT) " seconds. Quitting server.\n");
            active = false;
        } else {
            // loop dplist to to handle the sockets
            for (iterator_t it = it_begin(list); !it_is_null(&it); it_incr(&it)) {
                tcpsock_t* socket = it_get_element(&it);
                if (it.index != 0 && time(NULL) > *tcp_last_seen(socket) + TIMEOUT) {
                    printf("Sensor with id %d timed out. \n", *tcp_last_seen_sensor_id(socket));
                    dpl_remove_at_iterator(list, &it, true);
                } else if ((fds[it.index].revents & POLLIN) != 0) {
                    *tcp_last_seen(socket) = time(NULL);
                    if (it.index == 0) { // a new sensor is connected
                        tcpsock_t* new_socket = NULL;
                        tcp_wait_for_connection(socket, &new_socket);
                        dpl_insert_at_index(list, new_socket, dpl_size(list), false);
                        it_invalidate(&it);

                    } else { // data from existent connection is obtained
                        sensor_data_t data;
                        int bytes = sizeof(data.id);
                        tcp_receive(socket, &data.id, &bytes);

                        bytes = sizeof(data.value);
                        tcp_receive(socket, &data.value, &bytes);

                        bytes = sizeof(data.ts);
                        const int result = tcp_receive(socket, &data.ts, &bytes);

                        if (!socket->announced) {
                            printf("A new sensor with id = %" PRIu16 " has opened a new connection\n", data.id);
                            socket->announced = true;
                        }

                        if ((result == TCP_NO_ERROR) && bytes) {
                            *tcp_last_seen_sensor_id(socket) = data.id;
#if DEBUG
                            ASSERT_ELSE_PERROR(write(fd, &data.id, sizeof(data.id)) == sizeof(data.id));
                            ASSERT_ELSE_PERROR(write(fd, &data.value, sizeof(data.value)) == sizeof(data.value));
                            ASSERT_ELSE_PERROR(write(fd, &data.ts, sizeof(data.ts)) == sizeof(data.ts));
#endif
                            printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value, data.ts);
                            sbuffer_lock(buffer);
                            if (sbuffer_insert_first(buffer, &data) == SBUFFER_FAILURE) {
                                active = false;
                                it_invalidate(&it);
                            }
                            sbuffer_unlock(buffer);

                        } else if (result == TCP_CONNECTION_CLOSED) {
                            printf("Sensor with id %" PRIu16 " disconnected\n", *tcp_last_seen_sensor_id(socket));
                            dpl_remove_at_iterator(list, &it, true);
                        }
                    }
                }
            }
        }
    }
    free(fds);
#if DEBUG
    close(fd);
#endif
    dpl_free(&list, true);
}
