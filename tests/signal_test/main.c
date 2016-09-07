#include <uv.h>


uv_signal_t sig;


void on_signal(uv_signal_t *handle, int signal)
{
	printf("ReceiveSignal:%d\n", signal);

	
}

int main()
{
	uv_signal_init(uv_default_loop(), &sig);
	uv_signal_start(&sig, on_signal, SIGTERM);


	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
