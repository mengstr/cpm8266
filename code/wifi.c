 #include "c_types.h"
 #include "os_type.h"

void ets_timer_arm_new(volatile os_timer_t *a, int b, int c, int isMstimer);
void ets_timer_disarm(volatile os_timer_t *a);
void ets_timer_setfn(volatile os_timer_t *t, ETSTimerFunc *fn, void *parg);
void ets_isr_attach(int intr, void *handler, void *arg);
void ets_isr_mask(unsigned intr);
void ets_isr_unmask(unsigned intr);
void ets_delay_us(int ms);
void ets_update_cpu_frequency(int freqmhz);
void *ets_memcpy(void *dest, const void *src, size_t n);
void *ets_memset(void *s, int c, size_t n);
int os_printf(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));
int os_snprintf(char *str, size_t size, const char *format, ...) __attribute__ ((format (printf, 3, 4)));
int os_printf_plus(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));

#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "gpio.h"
#include "user_interface.h"
#include "user_config.h"
#include "gpio16.h"
#include "wifi.h"


  // gpio16_output_set(0);	
  // os_delay_us(250000);
  // gpio16_output_set(1);	

#include "espconn.h"

static struct espconn server;
static esp_tcp tcpconfig;

static void ICACHE_FLASH_ATTR server_recv_cb(void *arg, char *data, unsigned short len) {
  struct espconn *conn=(struct espconn *)arg;
  os_printf("recv_cb(len=%d)\r\n",len);
}
  
static void ICACHE_FLASH_ATTR server_discon_cb(void *arg) {
  os_printf("discon_cb() heap=%d\r\n",system_get_free_heap_size());
}
 
static void ICACHE_FLASH_ATTR server_sent_cb(void *arg) {
  os_printf("sent_cb()\r\n");
}
 
static void ICACHE_FLASH_ATTR server_connected_cb(void *arg) {
  os_printf("connected_cb() heap=%d\r\n",system_get_free_heap_size());
  struct espconn *conn=arg;

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
  espconn_send(conn,iac,12);
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

  gpio16_output_conf();

  os_printf("NONOS SDK version:%s\n", system_get_sdk_version());
  system_print_meminfo();


  gpio16_output_set(1);	
  wifi_set_opmode(STATION_MODE);
  os_memcpy(&stationConf.ssid, ssid, 32);
  os_memcpy(&stationConf.password, password, 32);
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

