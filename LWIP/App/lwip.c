/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * File Name          : LWIP.c
  * Description        : This file provides initialization code for LWIP
  *                      middleWare.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "lwip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/ip4_addr.h"
#include "lwip/dhcp.h"
#if defined ( __CC_ARM )  /* MDK ARM Compiler */
#include "lwip/sio.h"
#endif /* MDK ARM Compiler */
#include "ethernetif.h"

/* USER CODE BEGIN 0 */
#define FORCE_STATIC_NETWORK 1U
#define DHCP_FALLBACK_TIMEOUT_MS 15000U
#define FALLBACK_IP_ADDR0 192U
#define FALLBACK_IP_ADDR1 168U
#define FALLBACK_IP_ADDR2 1U
#define FALLBACK_IP_ADDR3 222U
#define FALLBACK_NETMASK_ADDR0 255U
#define FALLBACK_NETMASK_ADDR1 255U
#define FALLBACK_NETMASK_ADDR2 255U
#define FALLBACK_NETMASK_ADDR3 0U
#define FALLBACK_GW_ADDR0 192U
#define FALLBACK_GW_ADDR1 168U
#define FALLBACK_GW_ADDR2 1U
#define FALLBACK_GW_ADDR3 1U

/* USER CODE END 0 */
/* Private function prototypes -----------------------------------------------*/
static void ethernet_link_status_updated(struct netif *netif);
static void ethernet_status_updated(struct netif *netif);
static void Ethernet_Link_Periodic_Handle(struct netif *netif);
static void Ethernet_DHCP_Fallback_Handle(struct netif *netif);
/* ETH Variables initialization ----------------------------------------------*/
void Error_Handler(void);

/* DHCP Variables initialization ---------------------------------------------*/
uint32_t DHCPfineTimer = 0;
uint32_t DHCPcoarseTimer = 0;
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
uint32_t EthernetLinkTimer;

/* Variables Initialization */
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;

/* USER CODE BEGIN 2 */
static uint32_t ethernet_start_tick;
static uint8_t ethernet_fallback_configured;

/* USER CODE END 2 */

/**
  * LwIP initialization function
  */
void MX_LWIP_Init(void)
{
  /* Initilialize the LwIP stack without RTOS */
  lwip_init();

#if FORCE_STATIC_NETWORK
  IP4_ADDR(&ipaddr, FALLBACK_IP_ADDR0, FALLBACK_IP_ADDR1, FALLBACK_IP_ADDR2, FALLBACK_IP_ADDR3);
  IP4_ADDR(&netmask, FALLBACK_NETMASK_ADDR0, FALLBACK_NETMASK_ADDR1, FALLBACK_NETMASK_ADDR2, FALLBACK_NETMASK_ADDR3);
  IP4_ADDR(&gw, FALLBACK_GW_ADDR0, FALLBACK_GW_ADDR1, FALLBACK_GW_ADDR2, FALLBACK_GW_ADDR3);
#else
  /* IP addresses initialization with DHCP (IPv4) */
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;
#endif

  /* add the network interface (IPv4/IPv6) without RTOS */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /* Registers the default network interface */
  netif_set_default(&gnetif);

#if FORCE_STATIC_NETWORK
  netif_set_up(&gnetif);
  netif_set_link_up(&gnetif);
  printf("Ethernet static IP: %s\r\n", ip4addr_ntoa(netif_ip4_addr(&gnetif)));
#else
  if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called */
    netif_set_up(&gnetif);
  }
  else
  {
    /* When the netif link is down this function must be called */
    netif_set_down(&gnetif);
  }
#endif

  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&gnetif, ethernet_link_status_updated);
  netif_set_status_callback(&gnetif, ethernet_status_updated);

  /* Create the Ethernet link handler thread */

  /* Start DHCP negotiation for a network interface (IPv4) */
  ethernet_start_tick = HAL_GetTick();
  ethernet_fallback_configured = 0;
#if !FORCE_STATIC_NETWORK
  dhcp_start(&gnetif);
#endif

/* USER CODE BEGIN 3 */

/* USER CODE END 3 */
}

#ifdef USE_OBSOLETE_USER_CODE_SECTION_4
/* Kept to help code migration. (See new 4_1, 4_2... sections) */
/* Avoid to use this user section which will become obsolete. */
/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
#endif

/**
  * @brief  Ethernet Link periodic check
  * @param  netif
  * @retval None
  */
static void Ethernet_Link_Periodic_Handle(struct netif *netif)
{
/* USER CODE BEGIN 4_4_1 */
/* USER CODE END 4_4_1 */

  /* Ethernet Link every 100ms */
  if (HAL_GetTick() - EthernetLinkTimer >= 100)
  {
    EthernetLinkTimer = HAL_GetTick();
    ethernet_link_check_state(netif);
  }
/* USER CODE BEGIN 4_4 */
/* USER CODE END 4_4 */
}

/**
 * ----------------------------------------------------------------------
 * Function given to help user to continue LwIP Initialization
 * Up to user to complete or change this function ...
 * Up to user to call this function in main.c in while (1) of main(void)
 *-----------------------------------------------------------------------
 * Read a received packet from the Ethernet buffers
 * Send it to the lwIP stack for handling
 * Handle timeouts if LWIP_TIMERS is set and without RTOS
 * Handle the llink status if LWIP_NETIF_LINK_CALLBACK is set and without RTOS
 */
void MX_LWIP_Process(void)
{
/* USER CODE BEGIN 4_1 */
/* USER CODE END 4_1 */
  ethernetif_input(&gnetif);

/* USER CODE BEGIN 4_2 */
/* USER CODE END 4_2 */
  /* Handle timeouts */
  sys_check_timeouts();

#if !FORCE_STATIC_NETWORK
  Ethernet_Link_Periodic_Handle(&gnetif);
  Ethernet_DHCP_Fallback_Handle(&gnetif);
#endif

/* USER CODE BEGIN 4_3 */
/* USER CODE END 4_3 */
}

static void Ethernet_DHCP_Fallback_Handle(struct netif *netif)
{
  if (ethernet_fallback_configured || !netif_is_link_up(netif) || !ip4_addr_isany_val(*netif_ip4_addr(netif)))
  {
    return;
  }

  if (HAL_GetTick() - ethernet_start_tick < DHCP_FALLBACK_TIMEOUT_MS)
  {
    return;
  }

  dhcp_stop(netif);
  IP4_ADDR(&ipaddr, FALLBACK_IP_ADDR0, FALLBACK_IP_ADDR1, FALLBACK_IP_ADDR2, FALLBACK_IP_ADDR3);
  IP4_ADDR(&netmask, FALLBACK_NETMASK_ADDR0, FALLBACK_NETMASK_ADDR1, FALLBACK_NETMASK_ADDR2, FALLBACK_NETMASK_ADDR3);
  IP4_ADDR(&gw, FALLBACK_GW_ADDR0, FALLBACK_GW_ADDR1, FALLBACK_GW_ADDR2, FALLBACK_GW_ADDR3);
  netif_set_addr(netif, &ipaddr, &netmask, &gw);
  ethernet_fallback_configured = 1;
  printf("Ethernet DHCP timeout, fallback IP: %s\r\n", ip4addr_ntoa(netif_ip4_addr(netif)));
}

/**
  * @brief  Notify the User about the network interface config status
  * @param  netif: the network interface
  * @retval None
  */
static void ethernet_link_status_updated(struct netif *netif)
{
  if (netif_is_up(netif))
  {
/* USER CODE BEGIN 5 */
/* USER CODE END 5 */
  }
  else /* netif is down */
  {
/* USER CODE BEGIN 6 */
/* USER CODE END 6 */
  }
}

static void ethernet_status_updated(struct netif *netif)
{
  if (netif_is_up(netif) && !ip4_addr_isany_val(*netif_ip4_addr(netif)))
  {
    printf("Ethernet IP: %s\r\n", ip4addr_ntoa(netif_ip4_addr(netif)));
  }
}

#if defined ( __CC_ARM )  /* MDK ARM Compiler */
/**
 * Opens a serial device for communication.
 *
 * @param devnum device number
 * @return handle to serial device if successful, NULL otherwise
 */
sio_fd_t sio_open(u8_t devnum)
{
  sio_fd_t sd;

/* USER CODE BEGIN 7 */
  sd = 0; // dummy code
/* USER CODE END 7 */

  return sd;
}

/**
 * Sends a single character to the serial device.
 *
 * @param c character to send
 * @param fd serial device handle
 *
 * @note This function will block until the character can be sent.
 */
void sio_send(u8_t c, sio_fd_t fd)
{
/* USER CODE BEGIN 8 */
/* USER CODE END 8 */
}

/**
 * Reads from the serial device.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received - may be 0 if aborted by sio_read_abort
 *
 * @note This function will block until data can be received. The blocking
 * can be cancelled by calling sio_read_abort().
 */
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len)
{
  u32_t recved_bytes;

/* USER CODE BEGIN 9 */
  recved_bytes = 0; // dummy code
/* USER CODE END 9 */
  return recved_bytes;
}

/**
 * Tries to read from the serial device. Same as sio_read but returns
 * immediately if no data is available and never blocks.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received
 */
u32_t sio_tryread(sio_fd_t fd, u8_t *data, u32_t len)
{
  u32_t recved_bytes;

/* USER CODE BEGIN 10 */
  recved_bytes = 0; // dummy code
/* USER CODE END 10 */
  return recved_bytes;
}
#endif /* MDK ARM Compiler */

