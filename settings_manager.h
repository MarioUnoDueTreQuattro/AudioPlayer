#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QSettings>
#include <QString>
#include <QVariant>

/**
 * @brief The SettingsManager class implements a thread-safe Singleton
 * pattern to manage application settings using QSettings.
 */
class SettingsManager : public QObject
{
    Q_OBJECT

public:
    static SettingsManager *instance();
    // --- Configuration Access Methods ---
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setValue(const QString& key, const QVariant& value);
    void sync();

    // --- NEW: Scoped Access Methods ---

    /**
     * @brief Pushes a group name onto the settings path stack.
     * All subsequent read/write operations will be relative to this group.
     * @param prefix The group name to begin (e.g., "MainWindow").
     */
    void beginGroup(const QString& prefix);

    /**
     * @brief Pops the last group name off the path stack.
     * Reverts settings access to the previous scope.
     */
    void endGroup();
    QString settingsFilePath() const;
    const QSettings &getSettings() const;
private:
    QSettings m_settings;

    /**
     * @brief Private constructor to prevent direct instantiation.
     * @param parent The QObject parent (typically QCoreApplication::instance() for the Singleton).
     */
    SettingsManager(QObject* parent = nullptr);

    // Deleted copy constructor and assignment operator to enforce Singleton
    SettingsManager(const SettingsManager &) = delete;
    SettingsManager &operator=(const SettingsManager &) = delete;
    SettingsManager(SettingsManager &&) = delete;
    SettingsManager &operator=(SettingsManager &&) = delete;
};
#endif // SETTINGSMANAGER_H
