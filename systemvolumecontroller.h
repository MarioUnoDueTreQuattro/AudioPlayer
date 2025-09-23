#ifndef SYSTEMVOLUMECONTROLLER_H
#define SYSTEMVOLUMECONTROLLER_H

#include <QObject>
#include <Windows.h>
#include <Mmdeviceapi.h>
#include <Endpointvolume.h>

class SystemVolumeController : public QObject, public IAudioEndpointVolumeCallback
{
    Q_OBJECT

public:
    explicit SystemVolumeController(QObject *parent = nullptr);
    ~SystemVolumeController();

    float volume() const;     // 0.0 - 1.0
    bool isMuted() const;

public slots:
    void setVolume(float level);  // 0.0 - 1.0
    void mute(bool enable);
    void toggleMute();

signals:
    void volumeChanged(float newVolume);
    void muteStateChanged(bool muted);

private:
    void initialize();
    void cleanup();


    // COM callback interface
    STDMETHODIMP OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);

    // IUnknown methods required by COM
    STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    IAudioEndpointVolume *m_endpointVolume;
    LONG m_refCount;
};

#endif // SYSTEMVOLUMECONTROLLER_H
