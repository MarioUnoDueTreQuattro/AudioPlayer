#include "audio_fader.h"
#include <QDebug>

// Constructor: Initializes members and connects the timer
AudioFader::AudioFader(QMediaPlayer *player, QObject *parent)
    : QObject(parent),
      mediaPlayer(player),
      fadeTimer(new QTimer(this)),
      initialVolume(0),
      targetVolume(0),
      volumeStep(0),
      totalSteps(0),
      currentStep(0),
      m_bIsFading(false)/*,
      durationTimer(new QElapsedTimer())*/
{
    m_bEnableLog = true;
    // Connect the timer's timeout signal to the updateFade slot
    // This is the classic SIGNAL/SLOT approach
    fadeTimer->setTimerType(Qt::PreciseTimer);
    connect(fadeTimer, SIGNAL(timeout()), this, SLOT(updateFade()));
}

// Destructor: Ensure the timer is stopped to prevent calls during shutdown
AudioFader::~AudioFader()
{
    // It's critical to stop the timer here.
    // This prevents the updateFade() slot from being called while the
    // application's objects (like QMediaPlayer) are being destroyed.
    if (fadeTimer->isActive())
    {
        fadeTimer->stop();
    }
    QObject::disconnect(fadeTimer, SIGNAL(timeout()), this, SLOT(updateFade()));
    // Since fadeTimer has 'this' as its parent, Qt will delete it automatically.
    // if (durationTimer)
    // {
    // delete durationTimer;
    // durationTimer = nullptr;
    // }
    if (m_bEnableLog) qDebug() << __PRETTY_FUNCTION__;
}

// Public method to start the fade-in effect
void AudioFader::fadeIn(int targetVol, int durationMs)
{
    // Check if the player is valid
    if (!mediaPlayer) return;
    m_iDuration = durationMs;
    // Set the initial volume to 0 (silent) before starting,
    // in case it's not already.
    mediaPlayer->setVolume(0);
    //    // If the player is stopped, start playing (e.g., QMediaPlayer::PlayingState)
    // if (mediaPlayer->state() != QMediaPlayer::PlayingState) {
    // mediaPlayer->play();
    // }
    startFade(targetVol, durationMs);
    emit fadeProgressChanged(0);
}

// Public method to start the fade-out effect
void AudioFader::fadeOut(int durationMs)
{
    // Check if the player is valid
    if (!mediaPlayer) return;
    m_iDuration = durationMs;
    startFade(0, durationMs); // Fade out always targets volume 0
}

// Common function to initialize and start the fade logic
// audiofader.cpp

// Common function to initialize and start the fade logic
// audiofader.cpp - METODO startFade() RAFFORZATO

// Common function to initialize and start the fade logic
// audiofader.cpp

//void AudioFader::startFade(int targetVol, int durationMs)
//{
// if (!mediaPlayer) return;

// if (durationMs <= 0) {
//        // Instant volume change if duration is zero or less
// mediaPlayer->setVolume(targetVol);
// return;
// }

//    // 1. Initialize variables
// targetVolume = (float)targetVol;

//    // Inizializza l'accumulatore con il volume ATTUALE del player.
//    // Questo permette al fade di partire da dove si trova realmente l'audio.
// accumulatedVolume = (float)mediaPlayer->volume();

//    // 2. Calculation of total steps and step size
// const int updateIntervalMs = 20; // Good balance for smoothness (50 times/sec)

//    // Calcola il numero totale di step che avverranno
// float totalStepsFloat = (float)durationMs / (float)updateIntervalMs;

//    // Calcola il range di volume da coprire
// float volumeRange = targetVolume - accumulatedVolume;

//    // Calcola lo step: Range totale diviso per il numero totale di step
// volumeStep = volumeRange / totalStepsFloat;

//    // 3. Start the timer
// fadeTimer->setInterval(updateIntervalMs);
// fadeTimer->start();

// qDebug() << "Fade started. Initial Acc. Volume:" << accumulatedVolume
// << "Target:" << targetVol
// << "Total Steps:" << totalStepsFloat
// << "Step Size:" << volumeStep;
//}

void AudioFader::startFade(int targetVol, int durationMs)
{
    if (durationMs <= 0)
    {
        mediaPlayer->setVolume(targetVol);
        return;
    }
    m_iDuration = durationMs;
    //durationTimer->start();
    // Initialize state variables (now using float)
    // IMPORTANT: Cast to float for precision
    initialVolume = (float)mediaPlayer->volume();
    targetVolume = (float)targetVol;
    if (initialVolume==targetVol) return;
       if (!m_bIsFading)
    {
        emit fadeStarted();
        qDebug() << __PRETTY_FUNCTION__ << "fadeStarted ()";
    }
    else
    {
        qDebug() << __PRETTY_FUNCTION__ << "NOT fadeStarted ()";
    }
    m_bIsFading = true;
 currentStep = 0;
    // --- Calculation for smooth fade ---
    const int updateIntervalMs = 20;
    totalSteps = durationMs / updateIntervalMs;
    // Calculate how much volume to change per step (Volume is 0.0 - 100.0)
    float volumeRange = targetVolume - initialVolume;
    // Calculate the step size: total change / total steps
    // Ensure float division by casting totalSteps to float
    volumeStep = volumeRange / (float)totalSteps;
    // Set the timer interval and start it
    fadeTimer->setInterval(updateIntervalMs);
    fadeTimer->start();
    if (m_bEnableLog) qDebug() << "Fade started. Initial:" << initialVolume << "Target:" << targetVolume << "Step:" << volumeStep;
}

// SLOT: Called repeatedly by the QTimer
// audiofader.cpp - Modifica allo SLOT updateFade()

// audiofader.cpp - METODO updateFade()

// audiofader.cpp

//void AudioFader::updateFade()
//{
// if (!mediaPlayer) return;

//    // 1. Applica lo step all'accumulatore FLOAT
// accumulatedVolume += volumeStep;

//    // 2. Converi il volume float in un intero per QMediaPlayer
//    // Utilizza qRound() per un arrotondamento corretto (es. 0.5 diventa 1)
// int newVolume = qRound(accumulatedVolume);

//    // 3. Controlla il completamento
// bool finished = false;

//    // Se stiamo facendo fade IN (step positivo) e abbiamo raggiunto/superato il target
// if ((volumeStep > 0.0f && accumulatedVolume >= targetVolume) ||
//        // Se stiamo facendo fade OUT (step negativo) e abbiamo raggiunto/superato il target
// (volumeStep < 0.0f && accumulatedVolume <= targetVolume))
// {
// finished = true;
// }

//    // 4. Esegui l'azione finale
// if (finished)
// {
//        // Imposta il volume finale esatto e pulisci
// mediaPlayer->setVolume((int)targetVolume);
// finishFade();
// } else {
//        // Continua il fading: imposta il volume arrotondato.
// mediaPlayer->setVolume(newVolume);
// }

// qDebug() << "Fading... Current Volume:" << mediaPlayer->volume();
//}

void AudioFader::updateFade()
{
    currentStep++;
    // Calculate the new volume using floating point precision
    float newVolumeFloat;
    // Calculate the new volume based on the current step
    newVolumeFloat = initialVolume + (volumeStep * currentStep);
    // Convert the calculated float volume to the integer required by QMediaPlayer
    int newVolume = qRound(newVolumeFloat); // Use qRound for better accuracy than simple cast
    // Check if the fade is complete
    bool finished = false;
    // Condition 1: Check if we have passed the target volume
    if ((targetVolume > initialVolume && newVolume >= targetVolume) ||
        (targetVolume < initialVolume && newVolume <= targetVolume))
    {
        finished = true;
    }
    // Condition 2: Check if we have run out of steps (safety check)
    if (currentStep >= totalSteps)
    {
        finished = true;
    }
    if (finished)
    {
        // Reached the target, set final precise volume
        mediaPlayer->setVolume((int)targetVolume); // Set the final integer target volume
        finishFade();
        m_bIsFading = false;
        emit fadeFinished();
    }
    else
    {
        // Continue fading
        if (mediaPlayer != nullptr) mediaPlayer->setVolume(newVolume);
        emit fadeProgressChanged(newVolume);
    }
    // qDebug() << "Fading... Current Volume:" << mediaPlayer->volume();
}

// Stops the timer and performs cleanup tasks after the fade is complete
void AudioFader::finishFade()
{
    fadeTimer->stop();
    // delete fadeTimer;
    // fadeTimer=nullptr;
    // Specific action for fade OUT: stop the player when volume reaches 0
    // if (targetVolume == 0)
    // {
    // mediaPlayer->stop();
    // }
    // --- Misurazione e Output della Durata Totale ---
    // long long actualDuration = durationTimer->elapsed();
    // qDebug() << "Target Duration (ms):" << m_iDuration; // Stima della durata target
    // qDebug() << "Actual Duration (ms):" << actualDuration;
    if (m_bEnableLog) qDebug() << "Fade finished. Final Volume:" << mediaPlayer->volume();
    m_bIsFading = false;
    emit fadeFinished();
}

bool AudioFader::isFading() const
{
    return m_bIsFading;
}

void AudioFader::stopFadeImmediately()
{
    // fadeTimer->stop();
    // Esegue solo l'interruzione di sicurezza del timer.
    // Non esegue la logica di volume finale di finishFade().
    if (fadeTimer && fadeTimer->isActive())
    {
        fadeTimer->stop();
        if (m_bEnableLog) qDebug() << "Fade timer stopped by external request (shutdown).";
        m_bIsFading = false;
        emit fadeFinished();
    }
    QObject::disconnect(fadeTimer, SIGNAL(timeout()), this, SLOT(updateFade()));
    // Sicurezza aggiuntiva: Rendiamo il puntatore nullo DOPO la disconnessione
    // Sebbene non risolva la causa radice, previene riferimenti futuri.
    // Lo facciamo qui perché il genitore è l'unico a chiamare questa funzione prima della chiusura.
    mediaPlayer = nullptr;
}

void AudioFader::fadeToTarget(int targetVolume, int durationMs)
{
    m_iDuration = durationMs;
    // int iCurVol = mediaPlayer->volume ();
    startFade(targetVolume, durationMs);
}
