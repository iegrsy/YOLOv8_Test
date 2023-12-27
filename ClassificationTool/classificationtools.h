#ifndef CLASSIFICATIONTOOLS_H
#define CLASSIFICATIONTOOLS_H

#include <QMainWindow>
#include <QFileInfoList>
#include <QLabel>

#include <qfiledialog.h>

QT_BEGIN_NAMESPACE
namespace Ui { class ClassificationTools; }
QT_END_NAMESPACE

class ClassificationTools : public QMainWindow
{
	Q_OBJECT

public:
	ClassificationTools(QWidget *parent = nullptr);
	~ClassificationTools();

	void onUndoPressed();
	void onMovePressed();
	void onPreviousPressed();
	void onNextPressed();
public slots:
	void onClickSelectFolder1();
	void onClickSelectFolder2();
	void listFolderPaths1(QString path);
	void listFolderPaths2(QString path);

private:
	Ui::ClassificationTools *ui;

	QString workingPath = "/home/mert/Documents/person_filtered/person"; // QDir::homePath()
	QDir falsePositiveDir = QDir::cleanPath(workingPath + QDir::separator() + "false-positive");

	void listImages(QString path);

	int currentIndex = 0;
	QFileInfoList* activeFolderInfoList = nullptr;
	QFileInfoList folder1InfoList;
	QFileInfoList folder2InfoList;
	QString moveTargetDir;
};

#endif // CLASSIFICATION_H
