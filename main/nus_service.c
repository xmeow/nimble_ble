#include "esp_log.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "nus_service.h"

#define TAG "NUS_SERVICE"

// Define the 128-bit UUIDs for the NUS service and its characteristics
static const ble_uuid128_t nus_service_uuid =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e);
static const ble_uuid128_t nus_rx_characteristic_uuid =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e);
static const ble_uuid128_t nus_tx_characteristic_uuid =
    BLE_UUID128_INIT(0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e);

static uint16_t nus_service_handle;
static uint16_t nus_rx_char_handle;
static uint16_t nus_tx_char_handle;

static int nus_service_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg);

static uint16_t ble_uuid_128_to_16(const ble_uuid128_t *uuid128)
{
    uint16_t uuid16;
    uuid16 = (uuid128->value[12] << 8) | uuid128->value[13];
    return uuid16;
}

static const struct ble_gatt_chr_def nus_char_definitions[] = {
    {
        .uuid = &nus_rx_characteristic_uuid.u,
        .access_cb = nus_service_access_cb,
        .flags = BLE_GATT_CHR_F_WRITE,
    },
    {
        .uuid = &nus_tx_characteristic_uuid.u,
        .access_cb = nus_service_access_cb,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
    },
    {
        0, /* No more characteristics in this service */
    },
};

static const struct ble_gatt_svc_def nus_service_definition[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &nus_service_uuid.u,
        .characteristics = nus_char_definitions,
    },
    {
        0, /* No more services */
    },
};

void nus_init(void)
{
    int rc;

    rc = ble_gatts_count_cfg(nus_service_definition);
    assert(rc == 0);

    rc = ble_gatts_add_svcs(nus_service_definition);
    assert(rc == 0);
}

static int nus_service_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                                 struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    const ble_uuid128_t *uuid128;
    uint16_t uuid;
    int rc;

    // Define the 16-bit UUIDs for the NUS_RX and NUS_TX characteristics
    const uint16_t NUS_RX_CHARACTERISTIC_UUID_16 = 0x0002;
    const uint16_t NUS_TX_CHARACTERISTIC_UUID_16 = 0x0003;

    uuid128 = (const ble_uuid128_t *)ctxt->chr->uuid;
    uuid = ble_uuid_128_to_16(uuid128);

    if (uuid == NUS_RX_CHARACTERISTIC_UUID_16) {
        if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
            ESP_LOGI(TAG, "Received data: %.*s", ctxt->om->om_len, ctxt->om->om_data);
            return 0;
        }
    } else if (uuid == NUS_TX_CHARACTERISTIC_UUID_16) {
        if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
            const char *response = "Hello from ESP32!";
            rc = os_mbuf_append(ctxt->om, response, strlen(response));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
    }

    return BLE_ATT_ERR_UNLIKELY;
}
