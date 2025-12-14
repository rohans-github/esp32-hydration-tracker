#include "storage.h"
#include <Arduino.h>
#include <time.h>
#include <nvs_flash.h>
#include <nvs.h>

static const char* STORAGE_NAMESPACE = "hydrate";
Entry entries[MAX_ENTRIES]; //holds storage data

void storage_init() {
  nvs_flash_init();
  // Load existing data
  storage_load_entries(entries);
}

//loads past session data into entries array
void storage_load_entries(Entry entries[MAX_ENTRIES]) {
  nvs_handle_t handle;
  if (nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) {
    if (DEBUG) Serial.println("NVS open failed");
    return;
  }

  size_t required_size = sizeof(Entry) * MAX_ENTRIES;
  esp_err_t err = nvs_get_blob(handle, "history", entries, &required_size);

  if (err == ESP_ERR_NVS_NOT_FOUND) {
      // If no saved data, zero out
      memset(entries, 0, sizeof(Entry) * MAX_ENTRIES);
  }
  nvs_close(handle);
}

//saves entries array into memory
void storage_save_entries(const Entry entries[MAX_ENTRIES]) {
  nvs_handle_t handle;
  if (nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) {
    if (DEBUG) Serial.println("NVS open failed");
    return;
  }

  nvs_set_blob(handle, "history", entries, sizeof(Entry) * MAX_ENTRIES);
  nvs_commit(handle);
  nvs_close(handle);
}

//adds entry data to past session data and automatically saves to memory
void storage_add_entry(float grams_drank, float goal, float duration) {
  // Shift old entries: 6 <- 5 <- 4 ... <- 0
  for (int i = MAX_ENTRIES - 1; i > 0; i--) {
    entries[i] = entries[i - 1];
  }

  // Put new entry in position 0
  entries[0].grams_drank = grams_drank;
  entries[0].goal = goal;
  entries[0].duration = duration;

  // Save the updated array to memory
  storage_save_entries(entries);
}

//resets past session data
void storage_reset_entries() {
  for (int i = 0; i < MAX_ENTRIES; i++) {
    entries[i].grams_drank = 0;
    entries[i].goal = 0;
    entries[i].duration = 0;
  }
  storage_save_entries(entries);
}
