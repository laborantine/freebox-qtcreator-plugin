// Modified on 2026-11-11 for Qt Creator 20.0.2 and Qt 6.
#pragma once

#include <QMultiMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileInfo>

namespace Core { class IDocument; }

namespace Freebox {

class Project;

namespace Internal {

typedef struct {
    QString fileNameRelative;
    QString filePath;
    QFileInfo fileInfo;
    QByteArray content;
    bool toCheck;
} FreeStorePackageFile;

class FreeStorePackager : public QObject
{
    Q_OBJECT

public:
    FreeStorePackager(Freebox::Project *fbxProject, const QString &outFileName);
    bool operator()();
    QString outFileName() { return m_outFileName; }

private:
    bool checkSavedFiles(QList<Core::IDocument *> *failedToClose);
    bool isToCheck(const QString &filePath);
    bool preparePackaging();
    bool checkPackage();

    Freebox::Project *m_project;
    QString m_outFileName;
    bool m_seenManifest;

    QMultiMap<qsizetype, FreeStorePackageFile> m_packages;
    QStringList m_fileNameExists;

    QString m_projectDirPath;
    QList<Core::IDocument *> m_failedToClose;

};

} // namespace Internal
} // namespace Freebox

