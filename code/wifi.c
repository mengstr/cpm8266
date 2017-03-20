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

#include "conio.h"
#include "wifi.h"

void *ets_memcpy(void *dest, const void *src, size_t n);
void ets_update_cpu_frequency(int freqmhz);

  // gpio16_output_set(0);	
  // os_delay_us(250000);
  // gpio16_output_set(1);	


static struct espconn server;
static esp_tcp tcpconfig;

static void ICACHE_FLASH_ATTR server_recv_cb(void *arg, char *data, unsigned short len) {
  struct espconn *conn=(struct espconn *)arg;
//  ets_printf("recv_cb(len=%d)\r\n",len);
  for (int i=0; i<len; i++) {
    StoreInComBuf(data[i]);
  }
}
  
static void ICACHE_FLASH_ATTR server_discon_cb(void *arg) {
//  ets_printf("discon_cb() heap=%d\r\n",system_get_free_heap_size());
}
 
static void ICACHE_FLASH_ATTR server_sent_cb(void *arg) {
//  ets_printf("sent_cb()\r\n");
}
 
static volatile struct espconn *con;

static void ICACHE_FLASH_ATTR server_connected_cb(void *arg) {
//  ets_printf("connected_cb() heap=%d\r\n",system_get_free_heap_size());
  struct espconn *conn=arg;
  con=conn;

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


void TcpSend(char c) {
  espconn_send(con,&c,1);
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

  ets_install_putc1((void *)TcpSend);

  gpio16_output_set(1);	
  wifi_set_opmode(STATION_MODE);
  ets_memcpy(&stationConf.ssid, ssid, 32);
  ets_memcpy(&stationConf.password, password, 32);
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

