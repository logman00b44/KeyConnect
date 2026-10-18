#include "pch.h"
#include "HidDevice.h"
#include <cstring>

HidDevice::HidDevice()
    : m_magic(MAGIC), m_hLib(nullptr), m_dev(nullptr),
      m_hid_init(nullptr), m_hid_exit(nullptr), m_hid_open(nullptr),
      m_hid_close(nullptr), m_hid_get_feature_report(nullptr),
      m_hid_send_feature_report(nullptr), m_hid_error(nullptr)
{
}

HidDevice::~HidDevice()
{
    Close();
    FreeLibrary();
}

bool HidDevice::LoadLibrary()
{
    HMODULE hLib = ::LoadLibraryW(L"hidapi.dll");
    if (!hLib) {
        hLib = ::LoadLibraryW(L"driver\\host\\hidapi.dll");
    }
    if (!hLib) return false;

    m_hid_init = (int (*)(void))::GetProcAddress(hLib, "hid_init");
    m_hid_exit = (int (*)(void))::GetProcAddress(hLib, "hid_exit");
    m_hid_open = (void *(*)(unsigned short, unsigned short, const wchar_t *))::GetProcAddress(hLib, "hid_open");
    m_hid_close = (void (*)(void *))::GetProcAddress(hLib, "hid_close");
    m_hid_get_feature_report = (int (*)(void *, unsigned char *, size_t))::GetProcAddress(hLib, "hid_get_feature_report");
    m_hid_send_feature_report = (int (*)(void *, const unsigned char *, size_t))::GetProcAddress(hLib, "hid_send_feature_report");
    m_hid_error = (const wchar_t *(*)(void *))::GetProcAddress(hLib, "hid_error");

    if (!m_hid_init || !m_hid_exit || !m_hid_open || !m_hid_close ||
        !m_hid_get_feature_report || !m_hid_send_feature_report || !m_hid_error) {
        ::FreeLibrary(hLib);
        m_hLib = nullptr;
        return false;
    }

    m_hLib = hLib;
    return true;
}

void HidDevice::FreeLibrary()
{
    if (m_hLib) {
        ::FreeLibrary(m_hLib);
        m_hLib = nullptr;
        m_hid_init = nullptr;
        m_hid_exit = nullptr;
        m_hid_open = nullptr;
        m_hid_close = nullptr;
        m_hid_get_feature_report = nullptr;
        m_hid_send_feature_report = nullptr;
        m_hid_error = nullptr;
    }
}

std::wstring HidDevice::LastErrorStr()
{
    if (m_dev && m_hid_error) {
        const wchar_t *err = m_hid_error(m_dev);
        if (err) return err;
    }
    return L"unknown error";
}

bool HidDevice::Open()
{
    if (m_magic != MAGIC) {
        OutputDebugStringW(L"HidDevice::Open: MAGIC CORRUPTED! Cannot proceed.\n");
        return false;
    }
    {
        wchar_t buf[512];
        int n = swprintf(buf, 512,
            L"HidDevice::Open: magic=%llX hLib=%p dev=%p init=%p exit=%p open=%p close=%p get=%p send=%p err=%p\n",
            m_magic, m_hLib, m_dev,
            m_hid_init, m_hid_exit, m_hid_open, m_hid_close,
            m_hid_get_feature_report, m_hid_send_feature_report, m_hid_error);
        OutputDebugStringW(buf);
    }

    Close();
    if (!LoadLibrary()) return false;

    if (m_hid_init && m_hid_init() != 0) return false;

    m_dev = m_hid_open(VID, PID, nullptr);
    if (!m_dev) {
        m_hid_exit();
        return false;
    }
    return true;
}

void HidDevice::Close()
{
    if (m_dev && m_hid_close) {
        m_hid_close(m_dev);
        m_dev = nullptr;
    }
    if (m_hid_exit) {
        m_hid_exit();
    }
}

bool HidDevice::ReadKeymapEntry(int layer, int keyIdx, uint8_t *outMod, uint8_t *outKey)
{
    if (!m_dev || !m_hid_send_feature_report || !m_hid_get_feature_report)
        return false;

    unsigned char buf[5] = { 0x01, (unsigned char)layer, (unsigned char)keyIdx, 0, 0 };

    if (m_hid_send_feature_report(m_dev, buf, 5) < 0) {
        OutputDebugStringW(L"ReadKeymapEntry: SET_REPORT failed\n");
        return false;
    }

    unsigned char getBuf[5] = { 0x01, 0, 0, 0, 0 };
    if (m_hid_get_feature_report(m_dev, getBuf, 5) < 0) {
        OutputDebugStringW(L"ReadKeymapEntry: GET_REPORT failed\n");
        return false;
    }

    if (outMod) *outMod = getBuf[3];
    if (outKey) *outKey = getBuf[4];
    return true;
}

bool HidDevice::WriteKeymapEntry(int layer, int keyIdx, uint8_t mod, uint8_t key)
{
    if (!m_dev || !m_hid_send_feature_report)
        return false;

    unsigned char buf[5] = { 0x02, (unsigned char)layer, (unsigned char)keyIdx, mod, key };
    return m_hid_send_feature_report(m_dev, buf, 5) >= 0;
}

bool HidDevice::SaveToFlash()
{
    if (!m_dev || !m_hid_send_feature_report)
        return false;

    unsigned char buf[2] = { 0x03, 0xA5 };
    return m_hid_send_feature_report(m_dev, buf, 2) >= 0;
}

uint16_t HidDevice::GetMatrixState()
{
    if (!m_dev || !m_hid_get_feature_report)
        return 0xFFFF;

    unsigned char buf[3] = { 0x04, 0, 0 };
    if (m_hid_get_feature_report(m_dev, buf, 3) < 0)
        return 0xFFFF;

    return (uint16_t)buf[1] | ((uint16_t)buf[2] << 8);
}
