#include "systemvolumecontroller.h"
#include <QDebug>

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

STDMETHODIMP SystemVolumeController::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
{
    if (flow == eRender && role == eConsole)
    {
        QString deviceId = QString::fromWCharArray(pwstrDeviceId);
        if (deviceId == m_lastDeviceId)
            return S_OK; // Ignore duplicate notification for the same device
        m_lastDeviceId = deviceId;
        // cleanup();   // Release old device
        // initialize(); // Reinitialize with the new default
        qDebug() << "Default audio output device changed to:" << m_lastDeviceId;
        emit defaultDeviceChanged();
    }
    return S_OK;
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
