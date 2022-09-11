#pragma once

#include "DevControlCommand.h"

class ActivePowerControlCommand : public DevControlCommand {
public:
    ActivePowerControlCommand(uint64_t target_address = 0, uint64_t router_address = 0);

    virtual bool handleResponse(InverterAbstract* inverter, fragment_t fragment[], uint8_t max_fragment_id);
};