#pragma once
#include <windows.h>
#include <string>
#include <cstdint>

class HidDevice
{
public:
    HidDevice();
    ~HidDevice();

    bool Open();
    void Close();
    bool IsOpen() const { return m_dev != nullptr; }

    bool ReadKeymapEntry(int layer, int keyIdx, uint8_t *outMod, uint8_t *outKey);
    bool WriteKeymapEntry(int layer, int keyIdx, uint8_t mod, uint8_t key);
    bool SaveToFlash();
    uint16_t GetMatrixState();

private:
    bool LoadLibrary();
    void FreeLibrary();
    std::wstring LastErrorStr();

    static constexpr uint16_t VID = 0x2709;
    static constexpr uint16_t PID = 0x0001;
    static constexpr uint64_t MAGIC = 0x4D414749434F5253ull; // "MAGICORS"

    uint64_t m_magic = MAGIC;
    HMODULE m_hLib = nullptr;
    void *m_dev = nullptr;

    int (*m_hid_init)(void) = nullptr;
    int (*m_hid_exit)(void) = nullptr;
    void *(*m_hid_open)(unsigned short, unsigned short, const wchar_t *) = nullptr;
    void (*m_hid_close)(void *) = nullptr;
    int (*m_hid_get_feature_report)(void *, unsigned char *, size_t) = nullptr;
    int (*m_hid_send_feature_report)(void *, const unsigned char *, size_t) = nullptr;
    const wchar_t *(*m_hid_error)(void *) = nullptr;

};
