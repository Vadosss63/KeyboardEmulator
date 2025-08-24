#pragma once

#include <QImage>
#include <QtCore>

#include "Project.h"

namespace ProjectIO
{
bool save(const QString& filePath, const Project& prj);
bool load(const QString& filePath, Project& out);
}
