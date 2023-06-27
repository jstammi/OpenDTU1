// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "HM_Abstract.h"

class HM_2CH : public HM_Abstract {
public:
    explicit HM_2CH(HoymilesRadio* radio, uint64_t serial);
    static bool isValidSerial(uint64_t serial);
    String typeName();
    boolean verifyRxFragment(uint8_t fragmentCount, uint8_t fragmentId, uint8_t fragment[], uint8_t len);
    const byteAssign_t* getByteAssignment();
    uint8_t getByteAssignmentSize();
};