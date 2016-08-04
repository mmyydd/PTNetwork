#ifndef listener_h
#define listener_h

void listener_init(uv_loop_t *uv_loop, uint16_t port, uint32_t number_of_max_conn);
void listener_startup();
void listener_shutdown();
extern struct pt_server *listener;
#endif /* listener_h */
