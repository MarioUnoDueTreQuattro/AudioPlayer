#ifndef SYSTEMVOLUMECONTROLLER_H
#define SYSTEMVOLUMECONTROLLER_H

#include <QObject>
#include <Windows.h>
#include <Mmdeviceapi.h>
#include <Endpointvolume.h>
#include <QString>

class SystemVolumeController : public QObject,
                               public IAudioEndpointVolumeCallback,
                               public IMMNotificationClient
{
    Q_OBJECT

public:
    explicit SystemVolumeController(QObject *parent = nullptr);
    ~SystemVolumeController();

    float volume() const;     // 0.0 - 1.0
    bool isMuted() const;
    QString deviceId() const { return m_lastDeviceId; }
       QString deviceName() const { return m_deviceName; }
       QString deviceFriendlyName() const { return m_deviceFriendlyName; }
public slots:
    void setVolume(float level);  // 0.0 - 1.0
    void mute(bool enable);
    void toggleMute();
    void initialize();
    void cleanup();

signals:
    void volumeChanged(float newVolume);
    void muteStateChanged(bool muted);
 void defaultDeviceChanged(const QString &deviceId, const QString &friendlyName);
 void deviceInfoChanged(const QString &deviceId,
                               const QString &friendlyName);
private:
        QString friendlyNameForDefaultDevice();
 void updateCurrentDeviceInfo(IMMDevice *device);
    // COM callback interfaces
    STDMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override;
    STDMETHODIMP OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId) override;

    // Unused IMMNotificationClient methods
    STDMETHODIMP OnDeviceAdded(LPCWSTR) override { return S_OK; }
    STDMETHODIMP OnDeviceRemoved(LPCWSTR) override { return S_OK; }
    STDMETHODIMP OnDeviceStateChanged(LPCWSTR, DWORD) override { return S_OK; }
    STDMETHODIMP OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY) override { return S_OK; }

    // IUnknown methods (for both interfaces)
    STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;

    IAudioEndpointVolume *m_endpointVolume;
       IMMDeviceEnumerator *m_deviceEnumerator;
       QString m_lastDeviceId;
       QString m_deviceName;
       QString m_deviceFriendlyName;
       LONG m_refCount;
};

#endif // SYSTEMVOLUMECONTROLLER_H
