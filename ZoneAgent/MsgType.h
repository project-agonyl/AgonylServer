#pragma once
enum MessageType
{
    MSG_LS2ZA_USER_INFO_REQUEST = 0xE1,
    MSG_LS2ZA_USER_DROP_REQUEST = 0xE3,
    MSG_CL2ZA_USER_LOGIN_REQUEST = 0xE2,
    MSG_ZA2S_REQUEST = 0x01,
    MSG_CL2ZA_USER_PING_RESPONSE = 0xF0,
    MSG_CL2ZA_KH_CREST_REQUEST = 0x02,
    MSG_CL2ZA_CHECK_PAYMENT_INFO = 0x03
};