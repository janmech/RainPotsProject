#include "BoardManager.h"

BoardManager::BoardManager(char deviceName[], char manufacturer[])
{
    strcpy(m_device_name, deviceName);
    strcpy(m_manufacturer, manufacturer);
    uint8_t board_id[8];
    flash_get_unique_id(board_id);
    sprintf(m_bid, "%02x%02x%02x%02x%02x%02x%02x%02x", board_id[0], board_id[1], board_id[2], board_id[3], board_id[4], board_id[5], board_id[6], board_id[7]);

    sprintf(m_full_device_name, "%s (%s)", m_device_name, m_bid);
}

char *BoardManager::getBoardId()
{
    return m_bid;
}

char *BoardManager::getDeviceName()
{
    return m_device_name;
}

char *BoardManager::getDeviceFullName()
{
    return m_full_device_name;
}

char *BoardManager::getDeviceManufacturer()
{
    return m_manufacturer;
}