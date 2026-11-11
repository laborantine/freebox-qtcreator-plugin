// Modified on 2026-11-11 for Qt Creator 20.0.2 and Qt 6.
#include "freestorepackager.hh"
#include "project.hh"
#include "tar.hh"
#include "util/gzipper.hh"
#include "fileformat/manifest.hh"

#include <coreplugin/icore.h>
#include <coreplugin/idocument.h>
#include <coreplugin/documentmanager.h>
#include <qmljs/qmljsmodelmanagerinterface.h>

#include <QStringList>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>

namespace Freebox {
namespace Internal {

FreeStorePackager::FreeStorePackager(Freebox::Project *fbxProject, const QString &outFileName)
    : m_project(fbxProject),
      m_outFileName(outFileName),
      m_seenManifest(false)
{
}

bool FreeStorePackager::checkSavedFiles(QList<Core::IDocument *> *failedToClose)
{
    bool cancelledSave = false;
    bool alwaysSave = false;

    Core::DocumentManager::saveAllModifiedDocuments(
        QString(), &cancelledSave,
        QString(), &alwaysSave,
        failedToClose);

    return !cancelledSave;
}

bool FreeStorePackager::isToCheck(const QString &filePath)
{
    const auto path = Utils::FilePath::fromString(filePath);
    for (const Core::IDocument *document : std::as_const(m_failedToClose)) {
        if (document->filePath() == path)
            return false;
    }

    return true;
}

bool FreeStorePackager::preparePackaging()
{
    m_projectDirPath = m_project->projectDir().canonicalPath();
    const Utils::FilePaths paths = m_project->files(ProjectExplorer::Project::AllFiles);

    // Store file data and create virtual directories
    QSet<QString> seenDirs;
    const QDir root(m_projectDirPath);
    for (const Utils::FilePath &filePath : paths) {
        FreeStorePackageFile pf;

        pf.fileInfo = QFileInfo(filePath.toFSPathString());
        const QString cPath = pf.fileInfo.canonicalFilePath();
        const QString relative = root.relativeFilePath(pf.fileInfo.absoluteFilePath());
        const QString canonicalRelative = root.relativeFilePath(cPath);
        if (!pf.fileInfo.isFile() || cPath.isEmpty()
            || relative.startsWith(QStringLiteral("../")) || QDir::isAbsolutePath(relative)
            || canonicalRelative.startsWith(QStringLiteral("../")) || QDir::isAbsolutePath(canonicalRelative)) {
            QMessageBox::critical(Core::ICore::dialogParent(), tr("Make FreeStore Package Unsuccessful"),
                                  tr("Cannot package %1. All packaged files must exist in the project directory.")
                                      .arg(filePath.toUserOutput()));
            return false;
        }
        const QString cDir = pf.fileInfo.absolutePath();

        if (cDir != m_projectDirPath && !seenDirs.contains(cDir)) {
            seenDirs.insert(cDir);
            FreeStorePackageFile pfDir;
            pfDir.fileNameRelative = root.relativeFilePath(cDir) + u'/';
            pfDir.filePath = cDir;
            pfDir.fileInfo = QFileInfo(cDir);
            pfDir.toCheck = false; // virtual dirs are never parsed

            m_packages.insert(pfDir.fileNameRelative.size(), pfDir);
        }

        pf.fileNameRelative = relative;
        pf.filePath = cPath;
        pf.toCheck = isToCheck(cPath);

        QFile file(cPath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(Core::ICore::dialogParent(),
                                  tr("Make FreeStore Package Unsuccessful"),
                                  tr("Cannot open file '%1' for reading.")
                                  .arg(cPath));
            return false;
        }
        pf.content = file.readAll();
        file.close();

        m_packages.insert(pf.fileNameRelative.size(), pf);
        m_fileNameExists.append(pf.fileNameRelative);
    }

    return true;
}

bool FreeStorePackager::checkPackage()
{
    for (const FreeStorePackageFile &pf : m_packages.values()) {
        if (!pf.toCheck)
            continue;

        const auto path = Utils::FilePath::fromString(pf.filePath);
        const QmlJS::Dialect dialect = QmlJS::ModelManagerInterface::guessLanguageOfFile(path);

        bool checkingError = false;
        QString checkingErrorStr(tr("Syntax Error"));

        if (dialect.dialect() == QmlJS::Dialect::Json) {
            QJsonParseError jsError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(pf.content, &jsError);

            if ((checkingError = jsonDoc.isNull()))
                checkingErrorStr = jsError.errorString();
            else if (pf.fileNameRelative == QLatin1String("manifest.json")) {
                m_seenManifest = true;
                Freebox::Fileformat::Manifest man(jsonDoc);
                if ((checkingError = !man.isValid()))
                    checkingErrorStr = man.lastError();
                else {
                    QStringList requiredFiles = man.files();
                    QStringList missingFiles;
                    for (const QString &required : requiredFiles)
                        if (!m_fileNameExists.contains(required))
                            missingFiles.append(required);

                    if (!missingFiles.isEmpty()) {
                        checkingError = true;
                        checkingErrorStr = tr("The following files are missing or mispelled:");
                        for (const QString &missing : missingFiles)
                            checkingErrorStr += tr("<br>%1").arg(missing.toHtmlEscaped());
                    }
                }
            }
        } else if (dialect.isFullySupportedLanguage()) {
            auto doc = QmlJS::Document::create(path, dialect);
            doc->setSource(QString::fromUtf8(pf.content));

            switch (dialect.dialect()) {
            case QmlJS::Dialect::Qml:
            case QmlJS::Dialect::QmlQtQuick2:
                if (!doc->parseQml())
                    checkingError = true;
                break;
            case QmlJS::Dialect::JavaScript:
                if (!doc->parseJavaScript())
                    checkingError = true;
                break;
            default:
                break;
            }
        }

        if (checkingError) {
            QMessageBox::critical(Core::ICore::dialogParent(),
                                  tr("Make FreeStore Package Unsuccessful"),
                                  tr("Error checking file '%1': %2")
                                  .arg(pf.fileNameRelative)
                                  .arg(checkingErrorStr));
            return false;
        }
    }

    if (!m_seenManifest) {
        QMessageBox::critical(Core::ICore::dialogParent(),
                              tr("Make FreeStore Package Unsuccessful"),
                              tr("Missing file 'manifest.json'"));
        return false;
    }

    return true;
}

bool FreeStorePackager::operator()()
{
    MAN_ASSERT(checkSavedFiles(&m_failedToClose), return false);

    MAN_ASSERT(preparePackaging(), return false);

    MAN_ASSERT(checkPackage(), return false);

    Tar tar;
    QString invalidPath;
    QByteArray ba = tar.package(m_packages, &invalidPath);
    if (ba.isEmpty()) {
        QMessageBox::critical(Core::ICore::dialogParent(), tr("Make FreeStore Package Unsuccessful"),
                              tr("Cannot represent %1 in a FreeStore TAR archive.").arg(invalidPath));
        return false;
    }
    QString outFileName = QDir::toNativeSeparators(m_outFileName);

    QSaveFile outFile(outFileName);
    if (!outFile.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(Core::ICore::dialogParent(),
                              tr("Make FreeStore Package Unsuccessful"),
                              tr("Cannot open file '%1' for writing.")
                              .arg(outFileName));
        return false;
    }

    const QByteArray archive = Gzipper::gzip(ba);
    if (archive.isEmpty() || outFile.write(archive) != archive.size() || !outFile.commit()) {
        QMessageBox::critical(Core::ICore::dialogParent(),
                              tr("Make FreeStore Package Unsuccessful"),
                              tr("Cannot write to file '%1'.")
                              .arg(outFileName));
        return false;
    }

    return true;
}

} // namespace Internal
} // namespace Freebox
