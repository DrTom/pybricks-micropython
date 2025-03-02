#ifndef HCI_H
#define HCI_H

#include "hci_tl.h"

HCI_StatusCodes_t HCI_readBdaddr(void);
HCI_StatusCodes_t HCI_readLocalVersionInfo(void);
HCI_StatusCodes_t HCI_LE_readAdvertisingChannelTxPower(void);

#endif // HCI_H
