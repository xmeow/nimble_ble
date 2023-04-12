#ifndef NUS_SERVICE_H_
#define NUS_SERVICE_H_

#include "host/ble_hs.h"

void nus_init(void);
int nus_send_data(uint8_t *data, uint16_t len);

#endif // NUS_SERVICE_H_
