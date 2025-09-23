#include "systemvolumecontroller.h"
#include <QDebug>

SystemVolumeController::SystemVolumeController(QObject *parent)
    : QObject(parent),
      m_endpointVolume(nullptr)
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
    CoInitialize(nullptr); // Initialize COM

    IMMDeviceEnumerator *deviceEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
                          __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if (FAILED(hr)) {
        qWarning() << "Failed to create IMMDeviceEnumerator";
        return;
    }

    IMMDevice *defaultDevice = nullptr;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    deviceEnumerator->Release();

    if (FAILED(hr)) {
        qWarning() << "Failed to get default audio endpoint";
        return;
    }

    hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume),
                                 CLSCTX_INPROC_SERVER, nullptr, (LPVOID *)&m_endpointVolume);
    defaultDevice->Release();

    if (FAILED(hr)) {
        qWarning() << "Failed to activate IAudioEndpointVolume";
        m_endpointVolume = nullptr;
    }
}

void SystemVolumeController::cleanup()
{
    if (m_endpointVolume) {
        m_endpointVolume->Release();
        m_endpointVolume = nullptr;
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
    if (!m_endpointVolume)
        return;

    if (level < 0.0f) level = 0.0f;
    if (level > 1.0f) level = 1.0f;

    m_endpointVolume->SetMasterVolumeLevelScalar(level, nullptr);
    emit volumeChanged(level);
}

void SystemVolumeController::mute(bool enable)
{
    if (!m_endpointVolume)
        return;

    m_endpointVolume->SetMute(enable, nullptr);
    emit muteStateChanged(enable);
}

void SystemVolumeController::toggleMute()
{
    mute(!isMuted());
}
