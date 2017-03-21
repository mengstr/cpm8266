 #include "c_types.h"
 #include "os_type.h"


#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "gpio.h"
#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "gpio16.h"

#include "espincludes.h"

#include "machine.h"
#include "conio.h"
#include "wifi.h"

void system_soft_wdt_feed(void);
uint32_t system_get_time(void);
void *ets_memcpy(void *dest, const void *src, size_t n);
void ets_update_cpu_frequency(int freqmhz);

  // gpio16_output_set(0);	
  // os_delay_us(250000);
  // gpio16_output_set(1);	

#define procTaskPrio        0
#define procTaskQueueLen    1
static os_event_t    procTaskQueue[procTaskQueueLen];

static struct espconn *conn=NULL;
static struct espconn server;
static esp_tcp tcpconfig;

#define IOBUFSIZE   512

static volatile uint8_t needKickstart=true;
static volatile uint8_t txBuf[IOBUFSIZE];
static volatile uint16_t txR = 0;
static volatile uint16_t txW = 0;

static volatile uint32_t cnt;

//
//
//
static void ICACHE_FLASH_ATTR procTask(os_event_t *events) {
  system_os_post(procTaskPrio, 0, 0 );
  RunMachine(1);
}

//
//
//
uint16_t ICACHE_FLASH_ATTR GetTxCnt(void) {
  if (txW == txR) return 0;
  if (txW > txR) return txW - txR;
  return (sizeof(txBuf) - txR) + txW;
}

//
//
//
static void ICACHE_FLASH_ATTR server_recv_cb(void *arg, char *data, unsigned short len) {
  struct espconn *co=(struct espconn *)arg;
  for (int i=0; i<len; i++) {
    StoreInComBuf(data[i]);
  }
}
  
//
//
//
static void ICACHE_FLASH_ATTR server_discon_cb(void *arg) {
  conn=NULL;
}
 
//
//
//
static void ICACHE_FLASH_ATTR server_sent_cb(void *arg) {
  char buf[85];
  char len=0;

  if (txR==txW) needKickstart=1;

  while (txR!=txW && len<83) {
    txR = (txR + 1) & (sizeof(txBuf) - 1);
    buf[len++]=txBuf[txR];
  }
  if (conn && len) espconn_send(conn,buf,len);
}
 
//
//
//
static void ICACHE_FLASH_ATTR server_connected_cb(void *arg) {
  conn=arg;

  espconn_set_opt(conn, ESPCONN_REUSEADDR );
  espconn_set_opt(conn, ESPCONN_NODELAY);
  espconn_set_opt(conn, ESPCONN_KEEPALIVE);
  uint32_t keep_alive;
  keep_alive = 2;   //keep alive checking per 2s
  espconn_set_keepalive(conn, ESPCONN_KEEPIDLE, &keep_alive);
  keep_alive = 2; //repeat interval = 1s
  espconn_set_keepalive(conn, ESPCONN_KEEPINTVL, &keep_alive);
  keep_alive = 2;//repeat 2times
  espconn_set_keepalive(conn, ESPCONN_KEEPCNT, &keep_alive);

  espconn_regist_recvcb  (conn, server_recv_cb);
  espconn_regist_disconcb(conn, server_discon_cb);
  espconn_regist_sentcb  (conn, server_sent_cb);
 
  uint8_t iac []=
  "\377\373\003"    // IAC WILL SUPPRESS-GOAHEAD
	"\377\375\003"    // IAC DO SUPPRESS-GO-AHEAD
	"\377\373\001"    // IAC WILL SUPPRESS-ECHO
	"\377\375\001";   // IAC DO SUPPRESS-ECHO
  ets_printf(iac);

  ets_printf("\r\ncpm8266 - Z80 Emulator and CP/M 2.2 system version %d.%d\r\n",VERSION/10,VERSION%10);
  InitMachine();
  LoadBootSector();
}


void TcpSend(char ch) {
  if (conn) {
    if (GetTxCnt() < sizeof(txBuf) - 1) {
      txW = (txW + 1) & (sizeof(txBuf) - 1);
      txBuf[txW] = ch;
    }
    if (needKickstart) {
      needKickstart=0;
      server_sent_cb(NULL);
    }
  }
}


//
//
//
void	wifi_handle_event_cb(System_Event_t	*evt) {
//  os_printf("event	%x\n",	evt->event);
  switch	(evt->event)	{
    case	EVENT_STAMODE_CONNECTED:
//      os_printf("connect	to	ssid	%s,	channel	%d\n",evt->event_info.connected.ssid,	evt->event_info.connected.channel);
      gpio16_output_set(0);	
      break;
    case	EVENT_STAMODE_DISCONNECTED:
//      os_printf("disconnect	from	ssid	%s,	reason	%d\n", evt->event_info.disconnected.ssid,	evt->event_info.disconnected.reason);
      gpio16_output_set(0);	
      break;
    case	EVENT_STAMODE_AUTHMODE_CHANGE:
//      os_printf("mode:	%d	->	%d\n",	evt->event_info.auth_change.old_mode,	evt->event_info.auth_change.new_mode);
      break;
    case	EVENT_STAMODE_GOT_IP:
//      os_printf("ip:"	IPSTR	",mask:"	IPSTR	",gw:"	IPSTR, IP2STR(&evt->event_info.got_ip.ip),IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));
//      os_printf("\n");
      gpio16_output_set(1);	
      break;
    case	EVENT_SOFTAPMODE_STACONNECTED:
//      os_printf("station:	"	MACSTR	"join,	AID	=	%d\n",MAC2STR(evt->event_info.sta_connected.mac),	evt->event_info.sta_connected.aid);
      break;
    case	EVENT_SOFTAPMODE_STADISCONNECTED:
//      os_printf("station:	"	MACSTR	"leave,	AID	=	%d\n",MAC2STR(evt->event_info.sta_disconnected.mac),evt->event_info.sta_disconnected.aid);
      break;
    default:
      break;
  }
}




//
//
//
void ICACHE_FLASH_ATTR user_init() {
  struct station_config stationConf;
  const char ssid[32] = "01111523287";
  const char password[32] = "jamsheed12";

  // Bump up speed to 160MHz
  REG_SET_BIT(0x3ff00014, BIT(0));
  ets_update_cpu_frequency(160);
//  uart_div_modify(0, (160 * 1000000) / 115200);

  gpio16_output_conf();
  gpio16_output_set(0);	

  InitMachine();
 
  ets_install_putc1((void *)TcpSend);

  wifi_set_opmode(STATION_MODE);
  ets_memcpy(&stationConf.ssid, ssid, 32);
  ets_memcpy(&stationConf.password, password, 32);
  wifi_set_event_handler_cb(wifi_handle_event_cb);
  wifi_station_set_config(&stationConf);
  wifi_station_set_auto_connect(1);
  wifi_station_connect();

  espconn_tcp_set_max_con(1);
  ets_memset(&server, 0, sizeof(server));
  ets_memset(&tcpconfig, 0, sizeof(tcpconfig));
  espconn_create(&server);
  server.type=ESPCONN_TCP;
  server.state=ESPCONN_NONE;
  server.proto.tcp=&tcpconfig;
  tcpconfig.local_port=23;

  espconn_regist_connectcb(&server, server_connected_cb);
  espconn_accept(&server);
  espconn_regist_time(&server,3660,0);

  system_os_task(procTask, procTaskPrio, procTaskQueue, procTaskQueueLen);
  system_os_post(procTaskPrio, 0, 0 );
}

//
// Required for NONOS 2.0
//
uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void) {
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


//
// Required for NONOS 2.0
//
void ICACHE_FLASH_ATTR user_rf_pre_init(void) {
}

