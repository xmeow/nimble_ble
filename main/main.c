#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

// Include the custom UART service
#include "nus_service.h"

#define TAG "NUS_MAIN"

uint16_t nus_conn_handle;
uint8_t own_addr_type;
static const struct ble_gap_adv_params adv_params = {
    .conn_mode = BLE_GAP_CONN_MODE_UND,
    .disc_mode = BLE_GAP_DISC_MODE_GEN,
};

static void ble_on_reset(int reason)
{
    ESP_LOGE(TAG, "Resetting state; reason=%d", reason);
}


int ble_evt_cb(struct ble_gap_event *event, void *arg)
{
    int rc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            // Connection successful
            nus_conn_handle = event->connect.conn_handle;
            ESP_LOGI(TAG, "Connected to device");
        } else {
            // Connection failed; resume advertising
            ESP_LOGE(TAG, "Connection failed; status=%d", event->connect.status);
            rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_evt_cb, NULL);
            if (rc != 0) {
                ESP_LOGE(TAG, "Error enabling advertising; rc=%d", rc);
            }
        }
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        nus_conn_handle = 0;
        ESP_LOGI(TAG, "Disconnected from device");

        // Start advertising again
        rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_evt_cb, NULL);
        if (rc != 0) {
            ESP_LOGE(TAG, "Error enabling advertising; rc=%d", rc);
        }
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "Advertising complete");

        // Restart advertising
        rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_evt_cb, NULL);
        if (rc != 0) {
            ESP_LOGE(TAG, "Error enabling advertising; rc=%d", rc);
        }
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(TAG, "Subscribe event; conn_handle=%d value_handle=%d", event->subscribe.conn_handle,
                 event->subscribe.attr_handle);
        return 0;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "MTU updated: %d", event->mtu.value);
        return 0;

    default:
        break;
    }

    return 0;
}

static void ble_on_sync(void)
{
    int rc;

    // Retrieve the address type
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    assert(rc == 0);

    // Set the device name
    rc = ble_svc_gap_device_name_set("ESP32-NUS");
    assert(rc == 0);

    // Start advertising
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_evt_cb, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error enabling advertising; rc=%d", rc);
        return;
    }
}

static void start_ble_host(void)
{
    // Initialize the NimBLE host configuration
    ble_hs_cfg.sync_cb = ble_on_sync;
    ble_hs_cfg.reset_cb = ble_on_reset;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    // Register the GATT and GAP services
    ble_svc_gap_init();
    ble_svc_gatt_init();

    // Initialize and register the custom NUS service
    nus_init();

    // Initialize the NimBLE host task
    nimble_port_init();

    // Start the NimBLE host task
    nimble_port_freertos_init(start_ble_host);
}


void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize the ESP32 NimBLE
    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());
    nimble_port_init();

    // Start the NimBLE host
    start_ble_host();
}
