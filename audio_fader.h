#ifndef AUDIOFADER_H
#define AUDIOFADER_H

#include <QObject>
#include <QTimer>
#include <QMediaPlayer>
#include <QElapsedTimer>

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
    void fadeToTarget(int targetVolume, int durationMs);
    bool isFading() const;

private slots:
    // Slot connected to the QTimer timeout() signal
    void updateFade();
private:
    QMediaPlayer *mediaPlayer;
    QTimer *fadeTimer;
    //QElapsedTimer *durationTimer; // Timer per misurare la durata totale del fade
    float initialVolume;
    float targetVolume;
    float volumeStep;
    int totalSteps;
    int currentStep;
    int m_iDuration;
    //float accumulatedVolume;
    // Helper functions
    void startFade(int targetVolume, int durationMs);
    void finishFade();
    bool m_bEnableLog;
    bool m_bIsFading;

signals:
    void fadeProgressChanged(int volume);
    void fadeStarted();
    void fadeFinished();

};

#endif // AUDIOFADER_H
