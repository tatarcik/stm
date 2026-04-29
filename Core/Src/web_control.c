#include "web_control.h"

#include "lwip/tcp.h"
#include "main.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define WEB_CONTROL_PORT 80U
#define REQUEST_BUFFER_SIZE 768U
#define PAGE_BUFFER_SIZE 1792U
#define RESPONSE_BUFFER_SIZE 2304U

static struct tcp_pcb *web_control_pcb;
static char request_buffer[REQUEST_BUFFER_SIZE];
static char page_buffer[PAGE_BUFFER_SIZE];
static char response_buffer[RESPONSE_BUFFER_SIZE];
static volatile uint32_t sw1_exti_count;
static volatile uint32_t sw2_exti_count;
static volatile uint32_t sw3_exti_count;

void WebControl_RecordExti(uint16_t pin)
{
  if (pin == SW1_Pin)
  {
    sw1_exti_count++;
  }
  else if (pin == SW2_Pin)
  {
    sw2_exti_count++;
  }
  else if (pin == SW3_Pin)
  {
    sw3_exti_count++;
  }
}

static void WebControl_SetLed(uint8_t led, bool enabled)
{
  GPIO_PinState state = enabled ? GPIO_PIN_RESET : GPIO_PIN_SET;

  switch (led)
  {
    case 1:
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, state);
      break;
    case 2:
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, state);
      break;
    case 3:
      HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, state);
      break;
    default:
      break;
  }
}

static bool WebControl_IsLedOn(GPIO_TypeDef *port, uint16_t pin)
{
  return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET;
}

static const char *WebControl_OnOff(bool value)
{
  return value ? "ON" : "OFF";
}

static bool WebControl_QueryHas(const char *request, const char *key, const char *value)
{
  char token[32];
  snprintf(token, sizeof(token), "%s=%s", key, value);
  return strstr(request, token) != NULL;
}

static void WebControl_ApplyRequest(const char *request)
{
  if (WebControl_QueryHas(request, "led1", "on"))
  {
    WebControl_SetLed(1, true);
  }
  else if (WebControl_QueryHas(request, "led1", "off"))
  {
    WebControl_SetLed(1, false);
  }

  if (WebControl_QueryHas(request, "led2", "on"))
  {
    WebControl_SetLed(2, true);
  }
  else if (WebControl_QueryHas(request, "led2", "off"))
  {
    WebControl_SetLed(2, false);
  }

  if (WebControl_QueryHas(request, "led3", "on"))
  {
    WebControl_SetLed(3, true);
  }
  else if (WebControl_QueryHas(request, "led3", "off"))
  {
    WebControl_SetLed(3, false);
  }
}

static int WebControl_BuildLoginPage(void)
{
  return snprintf(page_buffer, sizeof(page_buffer),
                  "<!doctype html><html><head><meta charset=\"utf-8\">"
                  "<title>JZ-F407VET6 Control</title></head><body>"
                  "<h1>JZ-F407VET6 Control</h1>"
                  "<form method=\"get\" action=\"/enter.cgi\">"
                  "<label>Kullanici Adi</label><br>"
                  "<input name=\"user\" type=\"text\"><br>"
                  "<label>Sifre</label><br>"
                  "<input name=\"pwd\" type=\"password\"><br><br>"
                  "<input type=\"submit\" value=\"Giris\">"
                  "</form></body></html>");
}

static int WebControl_BuildSettingsPage(void)
{
  bool led1 = WebControl_IsLedOn(LED1_GPIO_Port, LED1_Pin);
  bool led2 = WebControl_IsLedOn(LED2_GPIO_Port, LED2_Pin);
  bool led3 = WebControl_IsLedOn(LED3_GPIO_Port, LED3_Pin);
  GPIO_PinState sw1 = HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin);
  GPIO_PinState sw2 = HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin);
  GPIO_PinState sw3 = HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin);
  uint32_t sw1_count = sw1_exti_count;
  uint32_t sw2_count = sw2_exti_count;
  uint32_t sw3_count = sw3_exti_count;

  return snprintf(page_buffer, sizeof(page_buffer),
                  "<!doctype html><html><head><meta charset=\"utf-8\">"
                  "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
                  "<title>JZ-F407VET6 Settings</title></head><body>"
                  "<h1>JZ-F407VET6 Settings</h1>"
                  "<p>LED1: %s | LED2: %s | LED3: %s</p>"
                  "<p>SW1: %s | SW2: %s | SW3: %s</p>"
                  "<p>EXTI counts - SW1: %lu | SW2: %lu | SW3: %lu</p>"
                  "<h2>LED Control</h2>"
                  "<p>LED1 <a href=\"/settings.shtml?led1=on\">ON</a> "
                  "<a href=\"/settings.shtml?led1=off\">OFF</a></p>"
                  "<p>LED2 <a href=\"/settings.shtml?led2=on\">ON</a> "
                  "<a href=\"/settings.shtml?led2=off\">OFF</a></p>"
                  "<p>LED3 <a href=\"/settings.shtml?led3=on\">ON</a> "
                  "<a href=\"/settings.shtml?led3=off\">OFF</a></p>"
                  "<p><a href=\"/settings.shtml?reset=1\">Reset EXTI counters</a></p>"
                  "<p><a href=\"/api/status\">JSON status</a></p>"
                  "<p><a href=\"/settings.shtml\">Refresh</a></p>"
                  "</body></html>",
                  WebControl_OnOff(led1), WebControl_OnOff(led2), WebControl_OnOff(led3),
                  sw1 == GPIO_PIN_SET ? "HIGH" : "LOW",
                  sw2 == GPIO_PIN_SET ? "HIGH" : "LOW",
                  sw3 == GPIO_PIN_SET ? "HIGH" : "LOW",
                  (unsigned long)sw1_count,
                  (unsigned long)sw2_count,
                  (unsigned long)sw3_count);
}

static int WebControl_BuildStatusJson(void)
{
  bool led1 = WebControl_IsLedOn(LED1_GPIO_Port, LED1_Pin);
  bool led2 = WebControl_IsLedOn(LED2_GPIO_Port, LED2_Pin);
  bool led3 = WebControl_IsLedOn(LED3_GPIO_Port, LED3_Pin);
  GPIO_PinState sw1 = HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin);
  GPIO_PinState sw2 = HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin);
  GPIO_PinState sw3 = HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin);

  return snprintf(page_buffer, sizeof(page_buffer),
                  "{\"led1\":\"%s\",\"led2\":\"%s\",\"led3\":\"%s\","
                  "\"sw1\":\"%s\",\"sw2\":\"%s\",\"sw3\":\"%s\","
                  "\"sw1_exti\":%lu,\"sw2_exti\":%lu,\"sw3_exti\":%lu}",
                  WebControl_OnOff(led1), WebControl_OnOff(led2), WebControl_OnOff(led3),
                  sw1 == GPIO_PIN_SET ? "HIGH" : "LOW",
                  sw2 == GPIO_PIN_SET ? "HIGH" : "LOW",
                  sw3 == GPIO_PIN_SET ? "HIGH" : "LOW",
                  (unsigned long)sw1_exti_count,
                  (unsigned long)sw2_exti_count,
                  (unsigned long)sw3_exti_count);
}

static int WebControl_BuildInfoPage(void)
{
  return snprintf(page_buffer, sizeof(page_buffer),
                  "<!doctype html><html><head><meta charset=\"utf-8\">"
                  "<title>Info</title></head><body>"
                  "<h1>JZ-F407VET6 Info</h1>"
                  "<p>Firmware: web control adaptation</p>"
                  "<p>Board: STM32F407VET6 Industrial Board</p>"
                  "<p><a href=\"/settings.shtml\">Settings</a></p>"
                  "</body></html>");
}

static int WebControl_BuildNotFoundPage(void)
{
  return snprintf(page_buffer, sizeof(page_buffer),
                  "<!doctype html><html><head><meta charset=\"utf-8\">"
                  "<title>404</title></head><body>"
                  "<h1>404 - Page not found</h1>"
                  "<p><a href=\"/index.html\">Home</a></p>"
                  "</body></html>");
}

static void WebControl_SendResponse(struct tcp_pcb *tpcb, const char *status, const char *content_type, int content_length)
{
  int response_length = snprintf(response_buffer, sizeof(response_buffer),
                                 "HTTP/1.0 %s\r\n"
                                 "Server: JZ-F407VET6\r\n"
                                 "Content-Type: %s\r\n"
                                 "Content-Length: %d\r\n"
                                 "Connection: close\r\n\r\n"
                                 "%s",
                                 status, content_type, content_length, page_buffer);

  if (response_length > 0)
  {
    tcp_write(tpcb, response_buffer, (u16_t)response_length, TCP_WRITE_FLAG_COPY);
  }
}

static void WebControl_ResetExtiCounters(void)
{
  sw1_exti_count = 0;
  sw2_exti_count = 0;
  sw3_exti_count = 0;
}

static err_t WebControl_Recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  (void)arg;

  if ((err != ERR_OK) || (p == NULL))
  {
    if (p != NULL)
    {
      pbuf_free(p);
    }
    tcp_close(tpcb);
    return ERR_OK;
  }

  uint16_t request_length = p->tot_len;
  if (request_length >= REQUEST_BUFFER_SIZE)
  {
    request_length = REQUEST_BUFFER_SIZE - 1U;
  }

  pbuf_copy_partial(p, request_buffer, request_length, 0);
  request_buffer[request_length] = '\0';
  tcp_recved(tpcb, p->tot_len);
  pbuf_free(p);

  const char *status = "200 OK";
  const char *content_type = "text/html; charset=utf-8";
  int content_length;

  if (strncmp(request_buffer, "GET / ", 6) == 0 || strncmp(request_buffer, "GET /index.html", 15) == 0)
  {
    content_length = WebControl_BuildLoginPage();
  }
  else if (strncmp(request_buffer, "GET /enter.cgi", 14) == 0)
  {
    if (WebControl_QueryHas(request_buffer, "user", "admin") && WebControl_QueryHas(request_buffer, "pwd", "signa"))
    {
      content_length = WebControl_BuildSettingsPage();
    }
    else
    {
      content_length = WebControl_BuildLoginPage();
    }
  }
  else if (strncmp(request_buffer, "GET /settings.shtml", 19) == 0 ||
           strncmp(request_buffer, "GET /control.cgi", 16) == 0 ||
           strncmp(request_buffer, "GET /deneme.cgi", 15) == 0)
  {
    if (WebControl_QueryHas(request_buffer, "reset", "1"))
    {
      WebControl_ResetExtiCounters();
    }
    WebControl_ApplyRequest(request_buffer);
    content_length = WebControl_BuildSettingsPage();
  }
  else if (strncmp(request_buffer, "GET /api/status", 15) == 0)
  {
    content_type = "application/json";
    content_length = WebControl_BuildStatusJson();
  }
  else if (strncmp(request_buffer, "GET /info.shtml", 15) == 0)
  {
    content_length = WebControl_BuildInfoPage();
  }
  else
  {
    status = "404 File not found";
    content_length = WebControl_BuildNotFoundPage();
  }

  if (content_length < 0)
  {
    status = "500 Internal Server Error";
    content_length = 0;
    page_buffer[0] = '\0';
  }

  WebControl_SendResponse(tpcb, status, content_type, content_length);
  tcp_close(tpcb);

  return ERR_OK;
}

static err_t WebControl_Accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  (void)arg;

  if (err != ERR_OK || newpcb == NULL)
  {
    return ERR_VAL;
  }

  tcp_recv(newpcb, WebControl_Recv);
  return ERR_OK;
}

void WebControl_Init(void)
{
  web_control_pcb = tcp_new();
  if (web_control_pcb == NULL)
  {
    return;
  }

  if (tcp_bind(web_control_pcb, IP_ADDR_ANY, WEB_CONTROL_PORT) != ERR_OK)
  {
    tcp_close(web_control_pcb);
    web_control_pcb = NULL;
    return;
  }

  web_control_pcb = tcp_listen(web_control_pcb);
  tcp_accept(web_control_pcb, WebControl_Accept);
}