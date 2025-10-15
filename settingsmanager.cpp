#include "settingsmanager.h"
#include <QCoreApplication>

// --- Private Constructor and Singleton Access (Unchanged) ---

SettingsManager::SettingsManager()
// Initialize QSettings using the organizationName and applicationName from qApp.
    : m_settings(QCoreApplication::organizationName(), QCoreApplication::applicationName())
{
}

SettingsManager &SettingsManager::instance()
{
    static SettingsManager s_instance;
    return s_instance;
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
