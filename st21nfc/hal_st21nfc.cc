/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *  Copyright (C) 2013 ST Microelectronics S.A.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  Modified by ST Microelectronics S.A. (adaptation of nfc_nci.c for ST21NFC
 *NCI version)
 *
 ******************************************************************************/

#include <cutils/properties.h>
#include <errno.h>
#include <hardware/nfc.h>
#include <string.h>

#include "android_logmsg.h"
#include "halcore.h"
#include "StNfc_hal_api.h"
#include "hal_config.h"

extern void HalCoreCallback(void* context, uint32_t event, const void* d,
                            size_t length);
extern bool I2cOpenLayer(void* dev, HAL_CALLBACK callb, HALHANDLE* pHandle);


typedef struct {
  struct nfc_nci_device nci_device;  // nci_device must be first struct member
  // below declarations are private variables within HAL
  nfc_stack_callback_t* p_cback;
  nfc_stack_data_callback_t* p_data_cback;
  HALHANDLE hHAL;
} st21nfc_dev_t;

const char* halVersion = "ST21NFC HAL1.1 Version 3.1.1";
uint8_t cmd_set_nfc_mode_enable[] = {0x2f, 0x02, 0x02, 0x02, 0x01};
uint8_t hal_is_closed = 1;
pthread_mutex_t hal_mtx = PTHREAD_MUTEX_INITIALIZER;
st21nfc_dev_t dev;
uint8_t hal_dta_state = 0;

using namespace android::hardware::nfc::V1_1;
using android::hardware::nfc::V1_1::NfcEvent;

/*
 * NCI HAL method implementations. These must be overridden
 */

extern bool hal_wrapper_open(st21nfc_dev_t* dev, nfc_stack_callback_t* p_cback,
                             nfc_stack_data_callback_t* p_data_cback,
                             HALHANDLE* pHandle);

extern int hal_wrapper_close(int call_cb);

int StNfc_hal_open(nfc_stack_callback_t* p_cback,
                    nfc_stack_data_callback_t* p_data_cback) {
  bool result = false;

  STLOG_HAL_D("HAL st21nfc: %s %s", __func__, halVersion);

  (void)pthread_mutex_lock(&hal_mtx);

  if (! hal_is_closed ) {
    hal_wrapper_close(0);
  }

  dev.p_cback = p_cback;
  dev.p_data_cback = p_data_cback;
  hal_dta_state = 0;
  // Initialize and get global logging level
  InitializeSTLogLevel();
  result = hal_wrapper_open(&dev, p_cback, p_data_cback, &(dev.hHAL));

  if (!result || !(dev.hHAL))
    {
      dev.p_cback(HAL_NFC_OPEN_CPLT_EVT, HAL_NFC_STATUS_FAILED);
      (void) pthread_mutex_unlock(&hal_mtx);
      return -1;  // We are doomed, stop it here, NOW !
    }
  hal_is_closed = 0;
  (void)pthread_mutex_unlock(&hal_mtx);
  return 0;
}

int StNfc_hal_write(uint16_t data_len,
                     const uint8_t* p_data) {
  STLOG_HAL_D("HAL st21nfc: %s", __func__);

  /* check if HAL is closed */
  int ret = (int)data_len;
  (void) pthread_mutex_lock(&hal_mtx);
  if (hal_is_closed)
    {
      ret = 0;
    }

  if (!ret)
    {
      (void) pthread_mutex_unlock(&hal_mtx);
      return ret;
    }
  if (!HalSendDownstream(dev.hHAL, p_data, data_len))
    {
      STLOG_HAL_E("HAL st21nfc %s  SendDownstream failed", __func__);
      (void) pthread_mutex_unlock(&hal_mtx);
      return 0;
    }
  (void) pthread_mutex_unlock(&hal_mtx);

  return ret;
}

int StNfc_hal_core_initialized( uint8_t* p_core_init_rsp_params) {
  STLOG_HAL_D("HAL st21nfc: %s", __func__);

  (void)pthread_mutex_lock(&hal_mtx);
  hal_dta_state = *p_core_init_rsp_params;
  dev.p_cback(HAL_NFC_POST_INIT_CPLT_EVT, HAL_NFC_STATUS_OK);
  (void) pthread_mutex_unlock(&hal_mtx);

  return 0;  // return != 0 to signal ready immediate
}

int StNfc_hal_pre_discover() {
  STLOG_HAL_D("HAL st21nfc: %s", __func__);

  return 0;  // false if no vendor-specific pre-discovery actions are needed
}

int StNfc_hal_close() {
  STLOG_HAL_D("HAL st21nfc: %s", __func__);

  /* check if HAL is closed */
  (void)pthread_mutex_lock(&hal_mtx);
  if ( hal_is_closed ) {
    (void)pthread_mutex_unlock(&hal_mtx);
    return 1;
  }
  if (hal_wrapper_close(1) == 0) {
    hal_is_closed = 1;
    (void)pthread_mutex_unlock(&hal_mtx);
    return 1;
  }
  hal_is_closed = 1;
  (void)pthread_mutex_unlock(&hal_mtx);

  hal_dta_state = 0;

  return 0;
}

int StNfc_hal_control_granted() {
  STLOG_HAL_D("HAL st21nfc: %s", __func__);

  return 0;
}

int StNfc_hal_power_cycle() {
  STLOG_HAL_D("HAL st21nfc: %s", __func__);

  /* check if HAL is closed */
  int ret = HAL_NFC_STATUS_OK;
  (void) pthread_mutex_lock(&hal_mtx);
  if (hal_is_closed)
    {
      ret = HAL_NFC_STATUS_FAILED;
    }

  if (ret != HAL_NFC_STATUS_OK)
    {
      (void) pthread_mutex_unlock(&hal_mtx);
      return ret;
    }
  dev.p_cback(HAL_NFC_OPEN_CPLT_EVT, HAL_NFC_STATUS_OK);

  (void) pthread_mutex_unlock(&hal_mtx);
  return HAL_NFC_STATUS_OK;
}


void StNfc_hal_factoryReset() {
  STLOG_HAL_D("HAL st21nfc: %s", __func__);
//Nothing needed for factory reset in st21nfc case.
}

int StNfc_hal_closeForPowerOffCase() {
  STLOG_HAL_D("HAL st21nfc: %s", __func__);
return StNfc_hal_close();
}

void StNfc_hal_getConfig(NfcConfig& config) {
  STLOG_HAL_D("HAL st21nfc: %s", __func__);
  unsigned long num = 0;
  std::array<uint8_t, 10> buffer;

  buffer.fill(0);
  long retlen = 0;

  memset(&config, 0x00, sizeof(NfcConfig));

  if (GetNumValue(NAME_POLL_BAIL_OUT_MODE, &num, sizeof(num))) {
    config.nfaPollBailOutMode = num;
  }
 
  if (GetNumValue(NAME_ISO_DEP_MAX_TRANSCEIVE, &num, sizeof(num))) {
    config.maxIsoDepTransceiveLength = num;
  }
  if (GetNumValue(NAME_DEFAULT_OFFHOST_ROUTE, &num, sizeof(num))) {
    config.defaultOffHostRoute = num;
    
  }
  if (GetNumValue(NAME_DEFAULT_NFCF_ROUTE, &num, sizeof(num))) {
    config.defaultOffHostRouteFelica = num;
  }
  if (GetNumValue(NAME_DEFAULT_SYS_CODE_ROUTE, &num, sizeof(num))) {
    config.defaultSystemCodeRoute = num;
  }
  if (GetNumValue(NAME_DEFAULT_SYS_CODE_PWR_STATE, &num, sizeof(num))) {
    config.defaultSystemCodePowerState = num;
  }
  if (GetNumValue(NAME_DEFAULT_ROUTE, &num, sizeof(num))) {
    config.defaultRoute = num;
  }
  if (GetByteArrayValue(NAME_DEVICE_HOST_WHITE_LIST, (char*)buffer.data(), buffer.size(), &retlen)) {
    config.hostWhitelist.resize(retlen);
    for(int i=0; i<retlen; i++) {
      config.hostWhitelist[i] = buffer[i];
  }
  }

  if (GetNumValue(NAME_OFF_HOST_ESE_PIPE_ID, &num, sizeof(num))) {
    config.offHostESEPipeId = num;
  }
  if (GetNumValue(NAME_OFF_HOST_SIM_PIPE_ID, &num, sizeof(num))) {
    config.offHostSIMPipeId = num;
  }
  if ((GetByteArrayValue(NAME_NFA_PROPRIETARY_CFG, (char*)buffer.data(), buffer.size(), &retlen))
         && (retlen == 9)) {
    config.nfaProprietaryCfg.protocol18092Active = (uint8_t) buffer[0];
    config.nfaProprietaryCfg.protocolBPrime = (uint8_t) buffer[1];
    config.nfaProprietaryCfg.protocolDual = (uint8_t) buffer[2];
    config.nfaProprietaryCfg.protocol15693 = (uint8_t) buffer[3];
    config.nfaProprietaryCfg.protocolKovio = (uint8_t) buffer[4];
    config.nfaProprietaryCfg.protocolMifare = (uint8_t) buffer[5];
    config.nfaProprietaryCfg.discoveryPollKovio = (uint8_t) buffer[6];
    config.nfaProprietaryCfg.discoveryPollBPrime = (uint8_t) buffer[7];
    config.nfaProprietaryCfg.discoveryListenBPrime = (uint8_t) buffer[8];
  } else {
    memset(&config.nfaProprietaryCfg, 0xFF, sizeof(ProtocolDiscoveryConfig));
  }
  if (GetNumValue(NAME_PRESENCE_CHECK_ALGORITHM, &num, sizeof(num))) {
      config.presenceCheckAlgorithm = (PresenceCheckAlgorithm)num;
  }
}
