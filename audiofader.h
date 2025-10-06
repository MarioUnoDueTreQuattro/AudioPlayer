#ifndef AUDIOFADER_H
#define AUDIOFADER_H

#include <QObject>
#include <QTimer>
#include <QMediaPlayer>

class AudioFader : public QObject
{
    Q_OBJECT

public:
    explicit AudioFader(QMediaPlayer *player, QObject *parent = nullptr);
    ~AudioFader() override;
    // Public methods for control
    void fadeIn(int targetVolume, int durationMs);
    void fadeOut(int durationMs);
    void stopFadeImmediately();
private slots:
    // Slot connected to the QTimer timeout() signal
    void updateFade();
private:
    QMediaPlayer *mediaPlayer;
    QTimer *fadeTimer;
    float initialVolume;
    float targetVolume;
    float volumeStep;
    int totalSteps;
    int currentStep;
    // Helper functions
    void startFade(int targetVolume, int durationMs);
    void finishFade();
};

#endif // AUDIOFADER_H
