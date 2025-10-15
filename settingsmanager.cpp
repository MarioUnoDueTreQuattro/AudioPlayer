#include "settingsmanager.h"
#include <QCoreApplication>

// --- Private Constructor and Singleton Access (Unchanged) ---

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent),
      m_settings(QCoreApplication::organizationName(), QCoreApplication::applicationName())
{
    // The parent is passed here only on the initial construction.
}

// --- Singleton Access Method Implementation (Updated to return pointer) ---

SettingsManager *SettingsManager::instance()
{
    /**
     * @brief Meyer's Singleton: Thread-safe initialization.
     * We pass the QCoreApplication instance as the parent, ensuring it's cleaned up
     * when the application shuts down, and placing it logically in the object tree.
     */
    static SettingsManager s_instance(QCoreApplication::instance());
    return &s_instance; // Return the address
}

// --- Configuration Access Methods Implementation (Unchanged) ---

QVariant SettingsManager::value(const QString& key, const QVariant& defaultValue) const
{
    return m_settings.value(key, defaultValue);
}

void SettingsManager::setValue(const QString& key, const QVariant& value)
{
    m_settings.setValue(key, value);
}

void SettingsManager::sync()
{
    m_settings.sync();
}

// --- NEW: Scoped Access Methods Implementation ---

void SettingsManager::beginGroup(const QString& prefix)
{
    /**
     * @brief Delegates the group start call to the internal QSettings object.
     * This establishes a scope for subsequent settings calls.
     */
    m_settings.beginGroup(prefix);
}

void SettingsManager::endGroup()
{
    /**
     * @brief Delegates the group end call to the internal QSettings object.
     * This closes the current settings scope.
     */
    m_settings.endGroup();
}

QString SettingsManager::settingsFilePath() const
{
    /**
     * @brief Delegates the call to the internal QSettings object to retrieve the storage path.
     * This path is dependent on the OS and the QSettings format (e.g., IniFormat, NativeFormat).
     *
     * On Windows, if the format is QSettings::NativeFormat,
     * the return value is a system registry path, not a file path.
     */
    return m_settings.fileName();
}
