#ifndef __WEB_CONTROL_H
#define __WEB_CONTROL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void WebControl_Init(void);
void WebControl_RecordExti(uint16_t pin);

#ifdef __cplusplus
}
#endif

#endif /* __WEB_CONTROL_H */