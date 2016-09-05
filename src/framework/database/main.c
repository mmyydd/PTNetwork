#include <common.h>
#include <commander.h>
#include <stdarg.h>
#include <time.h>
#include "main.h"
#include <qtime.h>
#include <file.h>
#include "daemon/daemon.h"
#include "daemon/signal.h"
#include "dispatch/listener.h"
#include "worker/logic.h"
#include "debug.h"
command_t cmd;

//const char *private_channel = "/var/tmp/private-database.sock";
//const char *public_channel = "/var/tmp/public-database.sock";
//const char *pid_file = "/var/tmp/pt_database.pid";
//const char *config_path = "network_config.dat";
//const char *log_dir = "/var/tmp/pt_service/";
//const char *db_config = "database_config.dat";


uv_signal_t sigint;
uv_signal_t sigterm;

int process_count = 20;
int server_type = SERVER_TYPE_DAEMON;
int queue_count = 10000;
qboolean is_signal = false;
const char *signal_cmd = "null";
void set_error_handler();

NetworkConfig *network_config;
DatabaseConfig *database_config;
const char *get_logfile()
{
	static char log[256];
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	sprintf(log,"%s%04d-%02d-%02d",LOG_DIR, timeinfo->tm_year + 1900,timeinfo->tm_mon + 1,timeinfo->tm_mday);

	return log;
}

///////////////////////////////////////////////////////////////////////////////////
//处理各种命令参数
///////////////////////////////////////////////////////////////////////////////////
void cmd_on_process_handler(struct command *self)
{
	process_count = atoi(self->arg);
	if(process_count == 0 || process_count > 100)
	{
		fprintf(stderr, "process count error\n");
		exit(1);
	}
}

void cmd_on_set_queue_handler(struct command *self)
{
	queue_count = atoi(self->arg);
	if(queue_count < 10000 || queue_count > 1000000)
	{
		fprintf(stderr, "queue_count is error\n");
		exit(1);
	}
}

void cmd_on_set_mode_server_handler(struct command *self)
{
	server_type = SERVER_TYPE_LISTENER;
}

void cmd_on_set_mode_logic_handler(struct command *self)
{
	server_type = SERVER_TYPE_LOGIC;
}

void cmd_on_set_debug_handler(struct command *self)
{
	server_type = SERVER_TYPE_DEBUG;
}

void cmd_on_signal_handler(struct command *self)
{
	is_signal = true;
	signal_cmd = self->arg;
}

void ERROR_Log(const char *message, const char *function, const char *file, int line)
{
	FILE *logFile;
	const char *logfile = get_logfile();

	logFile = fopen(logfile, "a+");

	if(logFile)
	{
		fprintf(logFile, "[ERROR][TRAP:%s,%s,%d]%s\n", function, file, line, message);
		fflush(logFile);
		fclose(logFile);
	}
}

void WARNING_Log(const char *message, const char *function, const char *file, int line)
{
	FILE *logFile;
	const char *logfile = get_logfile();

	logFile = fopen(logfile, "a+");

	if(logFile)
	{
		fprintf(logFile, "[WARNING][TRAP:%s,%s,%d]%s\n", function, file, line, message);
		fflush(logFile);
		fclose(logFile);
	}
}

void FATAL_Log(const char *message, const char *function, const char *file, int line)
{
	FILE *logFile;
	const char *logfile = get_logfile();

	logFile = fopen(logfile, "a+");

	if(logFile)
	{
		fprintf(logFile, "[FATAL][TRAP:%s,%s,%d]%s\n", function, file, line, message);
		fflush(logFile);
		fclose(logFile);
	}
}

void LOG_Log(const char *message, const char *function, const char *file, int line)
{
	FILE *logFile;
	const char *logfile = get_logfile();

	logFile = fopen(logfile, "a+");

	if(logFile)
	{
		fprintf(logFile, "[LOG]%s\n", message);
		fflush(logFile);
		fclose(logFile);
	}
}

void set_error_handler()
{
	set_error_report(ERROR_LEVEL_ERROR,ERROR_Log);
	set_error_report(ERROR_LEVEL_WARNING,WARNING_Log);
	set_error_report(ERROR_LEVEL_FATAL,FATAL_Log);
	set_error_report(ERROR_LEVEL_LOG,LOG_Log);
}

void load_network_config()
{
	unsigned char *buf;
	uint32_t len;
	if(!readfile(NETWORK_CONFIG, &buf, &len))
	{
		fprintf(stderr, "can not found %s\n", NETWORK_CONFIG);
		exit(1);
	}

	network_config = network_config__unpack(NULL, len, buf);
	MEM_FREE(buf);

	if(network_config == NULL)
	{
		fprintf(stderr, "parse network config failed\n");
		exit(1);
	}
}

void load_database_config()
{
	unsigned char *buf;
	uint32_t len;
	if(!readfile(DB_CONFIG, &buf, &len))
	{
		fprintf(stderr, "can not found %s\n", DB_CONFIG);
		exit(1);
	}
	database_config = database_config__unpack(NULL, len, buf);
	MEM_FREE(buf);

	if(database_config == NULL)
	{
		fprintf(stderr, "parse database config failed\n");
		exit(1);
	}
}

void load_server_config()
{
	load_network_config();
	load_database_config();
}

void parse_command(int argc,char *argv[])
{
	command_init(&cmd, APPLICATION_NAME, APPLICATION_VERSION);
	command_option(&cmd, "-n", "--process <count>", "设置逻辑进程的数量",cmd_on_process_handler);
	command_option(&cmd, "-q", "--queue <count>", "设置数据库请求队列的最大长度", cmd_on_set_queue_handler);
	command_option(&cmd, "-l", "--listener" , "[内部使用]启动处理网络连接的服务器", cmd_on_set_mode_server_handler);
	command_option(&cmd, "-w", "--logic", "[内部使用]逻辑进程使用的命令，连接调度服务器", cmd_on_set_mode_logic_handler);
	command_option(&cmd, "-v", "--no-daemon" , "调试模式启动服务器", cmd_on_set_debug_handler);
	command_option(&cmd, "-s","--signal <signal>","发送信号到daemon进程 支持信号:reload,quit",cmd_on_signal_handler);
	command_parse(&cmd, argc, (const char**)argv);
}

void on_signal_int(uv_signal_t *handle, int signal)
{
	
}


void sigint_start()
{
	uv_signal_start(&sigint, on_signal_int, SIGINT);
}

void sigint_stop()
{
	uv_signal_stop(&sigint);
}

int main(int argc,char *argv[])
{
	int r;
	
	parse_command(argc, argv);
	load_server_config();
	set_error_handler();

	uv_signal_init(uv_default_loop(), &sigint);
	uv_signal_init(uv_default_loop(), &sigterm);

	if(is_signal)
	{
		r = signal_main(argc, argv);
		goto End;
	}

	switch(server_type)
	{
		case SERVER_TYPE_DAEMON:
			r = daemon_main(argc, argv);
			break;
		case SERVER_TYPE_LOGIC:
			r = logic_main(argc, argv);
			break;
		case SERVER_TYPE_LISTENER:
			r = listener_main(argc, argv);
			break;
		case SERVER_TYPE_DEBUG:
			r = debug_main(argc, argv);
			break;
	}

	if(network_config)
	{
		network_config__free_unpacked(network_config, NULL);
		network_config = NULL;
	}
	
	if(database_config)
	{
		database_config__free_unpacked(database_config,NULL);
		database_config = NULL;
	}
End:
	return r;
}
