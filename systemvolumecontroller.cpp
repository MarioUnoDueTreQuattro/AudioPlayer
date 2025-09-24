#include "systemvolumecontroller.h"
#include <QDebug>
#ifndef INITGUID
#define INITGUID
#endif
#include <initguid.h>
#include <functiondiscoverykeys_devpkey.h>
SystemVolumeController::SystemVolumeController(QObject *parent)
    : QObject(parent),
      m_endpointVolume(nullptr),
      m_deviceEnumerator(nullptr),
      m_refCount(1)
{
    initialize();
}

SystemVolumeController::~SystemVolumeController()
{
    cleanup();
}

void SystemVolumeController::initialize()
{
    HRESULT hr;
    CoInitialize(nullptr);
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator), (LPVOID *)&m_deviceEnumerator);
    if (FAILED(hr))
    {
        qWarning() << "Failed to create IMMDeviceEnumerator";
        return;
    }
    // Register for default device change notifications
    hr = m_deviceEnumerator->RegisterEndpointNotificationCallback(this);
    if (FAILED(hr))
    {
        qWarning() << "Failed to register IMMNotificationClient";
    }
    // Get the default device and activate endpoint volume
    IMMDevice *defaultDevice = nullptr;
    hr = m_deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    if (FAILED(hr))
    {
        qWarning() << "Failed to get default audio endpoint";
        return;
    }
    // Store device info
        updateCurrentDeviceInfo(defaultDevice);
    hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume),
        CLSCTX_INPROC_SERVER, nullptr, (LPVOID *)&m_endpointVolume);
    defaultDevice->Release();
    if (FAILED(hr))
    {
        qWarning() << "Failed to activate IAudioEndpointVolume";
        m_endpointVolume = nullptr;
        return;
    }
    // Register for volume change notifications
    m_endpointVolume->RegisterControlChangeNotify(this);
 // Notify GUI that we have a device ready
    emit deviceInfoChanged(m_lastDeviceId, m_deviceFriendlyName);
}

void SystemVolumeController::cleanup()
{
    if (m_endpointVolume)
    {
        m_endpointVolume->UnregisterControlChangeNotify(this);
        m_endpointVolume->Release();
        m_endpointVolume = nullptr;
    }
    if (m_deviceEnumerator)
    {
        m_deviceEnumerator->UnregisterEndpointNotificationCallback(this);
        m_deviceEnumerator->Release();
        m_deviceEnumerator = nullptr;
    }
    CoUninitialize();
}

float SystemVolumeController::volume() const
{
    if (!m_endpointVolume)
        return 0.0f;
    float level = 0.0f;
    m_endpointVolume->GetMasterVolumeLevelScalar(&level);
    return level;
}

bool SystemVolumeController::isMuted() const
{
    if (!m_endpointVolume)
        return false;
    BOOL muted = FALSE;
    m_endpointVolume->GetMute(&muted);
    return muted;
}

void SystemVolumeController::setVolume(float level)
{
    if (m_endpointVolume)
    {
        if (level < 0.0f) level = 0.0f;
        if (level > 1.0f) level = 1.0f;
        m_endpointVolume->SetMasterVolumeLevelScalar(level, nullptr);
    }
}

void SystemVolumeController::mute(bool enable)
{
    if (m_endpointVolume)
    {
        m_endpointVolume->SetMute(enable, nullptr);
    }
}

void SystemVolumeController::toggleMute()
{
    mute(!isMuted());
}

// --- Callbacks ---

STDMETHODIMP SystemVolumeController::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
    if (!pNotify) return E_POINTER;
    emit volumeChanged(pNotify->fMasterVolume);
    emit muteStateChanged(pNotify->bMuted);
    return S_OK;
}

STDMETHODIMP SystemVolumeController::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR /*pwstrDeviceId*/)
{
    if (flow != eRender || role != eConsole)
        return S_OK;

    // Get friendly name and default device ID
    QString friendlyName = friendlyNameForDefaultDevice();
    // cleanup();   // Release old device
    // initialize(); // Reinitialize with the new default

    // Use the helper to get the actual device ID
    IMMDevice *defaultDevice = nullptr;
    QString deviceId;
    if (SUCCEEDED(m_deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice))) {
        LPWSTR id = nullptr;
        if (SUCCEEDED(defaultDevice->GetId(&id))) {
            deviceId = QString::fromWCharArray(id);
            CoTaskMemFree(id);
        }
        defaultDevice->Release();
    }

    // Ignore duplicates
    if (deviceId.isEmpty() || deviceId == m_lastDeviceId)
        return S_OK;

    m_lastDeviceId = deviceId;
    m_deviceFriendlyName = friendlyName;

    qDebug() << __PRETTY_FUNCTION__ << "New default device:" << deviceId;
    qDebug() << __PRETTY_FUNCTION__ << "Friendly name:" << friendlyName;

    emit defaultDeviceChanged(m_lastDeviceId, friendlyName);
    emit deviceInfoChanged(m_lastDeviceId, friendlyName);

    return S_OK;
}

//STDMETHODIMP SystemVolumeController::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
//{
//    if (flow == eRender && role == eConsole)
//    {
//        // Update internal info using the helper
//           QString friendlyName = friendlyNameForDefaultDevice();  // returns readable name
//           m_deviceFriendlyName = friendlyName;
//           QString deviceId = QString::fromWCharArray(pwstrDeviceId);
//        if (deviceId == m_lastDeviceId)
//            return S_OK; // Ignore duplicate notification for the same device
//        m_lastDeviceId = deviceId;
//        // cleanup();   // Release old device
//        // initialize(); // Reinitialize with the new default
//        // Also update device ID for reference
//        IMMDevice *defaultDevice = nullptr;
//        if (SUCCEEDED(m_deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice))) {
//            LPWSTR id = nullptr;
//            if (SUCCEEDED(defaultDevice->GetId(&id))) {
//                m_lastDeviceId = QString::fromWCharArray(id);
//                CoTaskMemFree(id);
//            }
//            defaultDevice->Release();
//        }

//        qDebug() << "New default device:" << friendlyName;
//               emit defaultDeviceChanged(m_lastDeviceId, friendlyName);
//                  emit deviceInfoChanged(m_lastDeviceId, friendlyName);
//               qDebug() << "Default audio output device changed to:" << m_lastDeviceId;
//               qDebug() << "Default audio output device changed to:" << friendlyName;
//    }
//    return S_OK;
//}

QString SystemVolumeController::friendlyNameForDefaultDevice()
{
    if (!m_deviceEnumerator)
        return QString();

    IMMDevice *defaultDevice = nullptr;
    if (FAILED(m_deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice)))
        return QString();

    QString friendlyName;

    // Try to get PROPVARIANT friendly name
    IPropertyStore *props = nullptr;
    if (SUCCEEDED(defaultDevice->OpenPropertyStore(STGM_READ, &props))) {
        PROPVARIANT varName;
        PropVariantInit(&varName);

        // Manual definition of PKEY_Device_FriendlyName
        PROPERTYKEY keyFriendlyName;
        keyFriendlyName.fmtid.Data1 = 0xa45c254e;
        keyFriendlyName.fmtid.Data2 = 0xdf1c;
        keyFriendlyName.fmtid.Data3 = 0x4efd;
        keyFriendlyName.fmtid.Data4[0] = 0x80;
        keyFriendlyName.fmtid.Data4[1] = 0x20;
        keyFriendlyName.fmtid.Data4[2] = 0x67;
        keyFriendlyName.fmtid.Data4[3] = 0xd1;
        keyFriendlyName.fmtid.Data4[4] = 0x46;
        keyFriendlyName.fmtid.Data4[5] = 0xa8;
        keyFriendlyName.fmtid.Data4[6] = 0x50;
        keyFriendlyName.fmtid.Data4[7] = 0x01;
        keyFriendlyName.pid = 14;

        if (SUCCEEDED(props->GetValue(keyFriendlyName, &varName))) {
            if (varName.vt == VT_LPWSTR && varName.pwszVal)
                friendlyName = QString::fromWCharArray(varName.pwszVal);
        }
        PropVariantClear(&varName);
        props->Release();
    }

    // If the friendly name is empty, try to generate a readable fallback
    if (friendlyName.isEmpty()) {
        LPWSTR id = nullptr;
        if (SUCCEEDED(defaultDevice->GetId(&id))) {
            QString deviceId = QString::fromWCharArray(id);
            CoTaskMemFree(id);

            // Show the last segment of GUID for readability
            int braceIndex = deviceId.lastIndexOf('}');
            if (braceIndex > 0 && braceIndex + 1 < deviceId.length())
                friendlyName = "Device " + deviceId.mid(braceIndex + 1);
            else
                friendlyName = "Device " + deviceId;
        } else {
            friendlyName = "Unknown Device";
        }
    }

    defaultDevice->Release();
    return friendlyName;
}

// --- COM interface management ---

STDMETHODIMP SystemVolumeController::QueryInterface(REFIID iid, void **ppvObject)
{
    if (iid == __uuidof(IUnknown) ||
        iid == __uuidof(IAudioEndpointVolumeCallback))
    {
        *ppvObject = static_cast<IAudioEndpointVolumeCallback *>(this);
    }
    else if (iid == __uuidof(IMMNotificationClient))
    {
        *ppvObject = static_cast<IMMNotificationClient *>(this);
    }
    else
    {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE SystemVolumeController::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

ULONG STDMETHODCALLTYPE SystemVolumeController::Release()
{
    LONG ref = InterlockedDecrement(&m_refCount);
    if (ref == 0) delete this;
    return ref;
}

void SystemVolumeController::updateCurrentDeviceInfo(IMMDevice *device)
{
    m_deviceName.clear();
    m_deviceFriendlyName.clear();

    if (!device)
        return;

    // Store Device ID
    LPWSTR id = nullptr;
    if (SUCCEEDED(device->GetId(&id))) {
        m_lastDeviceId = QString::fromWCharArray(id);
        CoTaskMemFree(id);
    }

    // Define PROPERTYKEY manually (same GUID as PKEY_Device_FriendlyName)
    PROPERTYKEY keyFriendlyName;
    keyFriendlyName.fmtid.Data1 = 0xa45c254e;
    keyFriendlyName.fmtid.Data2 = 0xdf1c;
    keyFriendlyName.fmtid.Data3 = 0x4efd;
    keyFriendlyName.fmtid.Data4[0] = 0x80;
    keyFriendlyName.fmtid.Data4[1] = 0x20;
    keyFriendlyName.fmtid.Data4[2] = 0x67;
    keyFriendlyName.fmtid.Data4[3] = 0xd1;
    keyFriendlyName.fmtid.Data4[4] = 0x46;
    keyFriendlyName.fmtid.Data4[5] = 0xa8;
    keyFriendlyName.fmtid.Data4[6] = 0x50;
    keyFriendlyName.fmtid.Data4[7] = 0x1;
    keyFriendlyName.pid = 14; // Friendly name property

    IPropertyStore *props = nullptr;
    if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &props))) {
        PROPVARIANT varName;
        PropVariantInit(&varName);

        if (SUCCEEDED(props->GetValue(keyFriendlyName, &varName))) {
            if (varName.vt == VT_LPWSTR && varName.pwszVal)
                m_deviceFriendlyName = QString::fromWCharArray(varName.pwszVal);
            PropVariantClear(&varName);
        }
        props->Release();
    }

    if (m_deviceFriendlyName.isEmpty()) {
        // Fallback to raw ID (not user-friendly but avoids crash)
        m_deviceFriendlyName = m_lastDeviceId;
    }
}
