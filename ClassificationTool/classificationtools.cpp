#include "classificationtools.h"
#include "ui_classificationtools.h"

#include <QDialog>
#include <QPushButton>
#include <QDebug>

#include <QFileDialog>
#include <QImageReader>
#include <QWidget>
#include <QKeyEvent>
#include <QKeySequence>
#include <QShortcut>

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

ClassificationTools::ClassificationTools(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::ClassificationTools)
{
	ui->setupUi(this);

	QObject::connect(ui->selectFolder1Btn, &QPushButton::clicked, this, &ClassificationTools::onClickSelectFolder1);
	QObject::connect(ui->selectFolder2Btn, &QPushButton::clicked, this, &ClassificationTools::onClickSelectFolder2);

	QObject::connect(ui->filePath1, &QLineEdit::textChanged, this, &ClassificationTools::listFolderPaths1);
	QObject::connect(ui->filePath2, &QLineEdit::textChanged, this, &ClassificationTools::listFolderPaths2);
	QObject::connect(ui->folder1ActionBtn, &QPushButton::clicked, this, [&](){
		//this->currentIndex = 0;
		this->activeFolderInfoList = &this->folder1InfoList;
		this->moveTargetDir = ui->filePath2->text();
		setPreviewImage();
		ui->frame_1->setStyleSheet("QFrame#frame_1 {border: 1px solid #0000FF}");
		ui->frame_2->setStyleSheet("");
	});
	QObject::connect(ui->folder2ActionBtn, &QPushButton::clicked, this, [&](){
		//this->currentIndex = 0;
		this->activeFolderInfoList = &this->folder2InfoList;
		this->moveTargetDir = ui->filePath1->text();
		setPreviewImage();
		ui->frame_1->setStyleSheet("");
		ui->frame_2->setStyleSheet("QFrame#frame_2 {border: 1px solid #0000FF}");
	});

	ui->filePath1->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->filePath2->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->selectFolder1Btn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->selectFolder2Btn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->folder1ActionBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->folder2ActionBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->previousBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->moveBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->nextBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	ui->undoBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);

	QShortcut *shortcutNext = new QShortcut(QKeySequence(Qt::Key_Right), this);
	QObject::connect(shortcutNext, &QShortcut::activated, this, &ClassificationTools::onNextPressed);
	QShortcut *shortcutPrevious = new QShortcut(QKeySequence(Qt::Key_Left), this);
	QObject::connect(shortcutPrevious, &QShortcut::activated, this, &ClassificationTools::onPreviousPressed);
	QShortcut *shortcutMove = new QShortcut(QKeySequence(Qt::Key_Space), this);
	QObject::connect(shortcutMove, &QShortcut::activated, this, &ClassificationTools::onMovePressed);
	QShortcut *shortcutUndo = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
	QObject::connect(shortcutUndo, &QShortcut::activated, this, &ClassificationTools::onUndoPressed);

	QObject::connect(ui->nextBtn, &QPushButton::pressed, this, &ClassificationTools::onNextPressed);
	QObject::connect(ui->previousBtn, &QPushButton::pressed, this, &ClassificationTools::onPreviousPressed);
	QObject::connect(ui->moveBtn, &QPushButton::pressed, this, &ClassificationTools::onMovePressed);
	QObject::connect(ui->undoBtn, &QPushButton::pressed, this, &ClassificationTools::onUndoPressed);

	if (!falsePositiveDir.exists()) {
		auto path = falsePositiveDir.absolutePath();
		if (!falsePositiveDir.mkpath(".")) {
			QString errMsg = QString("Error creating folder %1").arg(path);
			qDebug() << errMsg;
			QMessageBox::critical(this, "Create Folder", errMsg, QMessageBox::StandardButton::NoButton, QMessageBox::StandardButton::NoButton);
		}
	}

	loadConfig();
}

ClassificationTools::~ClassificationTools()
{
	delete ui;
}

void ClassificationTools::onClickSelectFolder1()
{
	QString dirName = QFileDialog::getExistingDirectory(this, "Open a folder", workingPath);
	ui->filePath1->setText(dirName);
}

void ClassificationTools::onClickSelectFolder2()
{
	QString dirName = QFileDialog::getExistingDirectory(this, "Open a folder", workingPath);
	ui->filePath2->setText(dirName);
}

void ClassificationTools::listFolderPaths1(QString path)
{
	this->folder1InfoList = QDir(path).entryInfoList(QDir::Files);
}

void ClassificationTools::listFolderPaths2(QString path)
{
	this->folder2InfoList = QDir(path).entryInfoList(QDir::Files);
}

void ClassificationTools::saveConfig()
{
	QJsonObject obj
	{
		{"currentIndex", this->currentIndex},
		{"folder1", ui->filePath1->text()},
		{"folder2", ui->filePath2->text()}
	};

	QJsonDocument doc;
	doc.setObject(obj);
	configFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
	configFile.write(doc.toJson());
	configFile.flush();
	configFile.close();

	qDebug() << "Saved config " << doc.toJson();
}

void ClassificationTools::loadConfig()
{
	configFile.open(QIODevice::OpenModeFlag::ReadOnly);
	QString config(configFile.readAll());
	configFile.flush();
	configFile.close();

	qDebug() << "Load config " << config;
	QJsonDocument doc = QJsonDocument::fromJson(config.toUtf8());
	QJsonObject obj = doc.object();
	this->currentIndex = obj["currentIndex"].toInt();
	ui->filePath1->setText(obj["folder1"].toString());
	ui->filePath2->setText(obj["folder2"].toString());

	ui->folder1ActionBtn->click();
}

void ClassificationTools::closeEvent(QCloseEvent *event)
{
	saveConfig();
}

void ClassificationTools::onNextPressed() {
	if (this->activeFolderInfoList == nullptr) {
		qDebug() << "Not set current folder !!!!!!";
		return;
	}

	auto it = this->activeFolderInfoList;
	this->currentIndex++;
	if (this->currentIndex >= it->size()) {
		this->currentIndex = it->size() - 1;
		QMessageBox::information(this, "", "Last image", QMessageBox::StandardButton::NoButton, QMessageBox::StandardButton::NoButton);
		return;
	}

	qDebug() << "next";
	setPreviewImage();
}

void ClassificationTools::onPreviousPressed() {
	this->currentIndex--;
	if (this->currentIndex < 0) {
		this->currentIndex = 0;
		QMessageBox::warning(this, "", "First image", QMessageBox::StandardButton::NoButton, QMessageBox::StandardButton::NoButton);
		return;
	}

	qDebug() << "pre";
	setPreviewImage();
}

void ClassificationTools::onMovePressed() {
	if (this->activeFolderInfoList == nullptr || moveTargetDir == nullptr || moveTargetDir.isEmpty()) {
		qDebug() << "Not selected move target !!!";
		return;
	}

	auto it = this->activeFolderInfoList;
	auto fi = it->at(this->currentIndex);
	QFile::copy(fi.absoluteFilePath(), falsePositiveDir.absoluteFilePath(fi.fileName()));
	auto isMove = QFile::rename(fi.absoluteFilePath(), QDir(moveTargetDir).absoluteFilePath(fi.fileName()));
	if (isMove) {
		qDebug() << "Moved file name: " << isMove << fi.absoluteFilePath() << " >> " << moveTargetDir;

		it->removeAt(this->currentIndex);

		{
			this->folder1InfoList = QDir(ui->filePath1->text()).entryInfoList(QDir::Files);
			this->folder2InfoList = QDir(ui->filePath2->text()).entryInfoList(QDir::Files);
		}
		onNextPressed();
	}
}

void ClassificationTools::setPreviewImage() {
	if (this->activeFolderInfoList == nullptr)
		return;

	if (this->activeFolderInfoList->empty()) {
		QMessageBox::critical(this, "", "Image not found", QMessageBox::StandardButton::NoButton, QMessageBox::StandardButton::NoButton);
		return;
	}
	if (this->currentIndex < 0 || this->activeFolderInfoList->size() <= this->currentIndex)
		return;

	ui->statusbar->showMessage(QString("%1/%2").arg(this->currentIndex).arg(this->activeFolderInfoList->size()));
	auto fi = this->activeFolderInfoList->at(this->currentIndex);
	ui->previewImage->setPixmap(QPixmap(fi.absoluteFilePath()));
}

void ClassificationTools::onUndoPressed() {
	qDebug() << "TODO: undo";
}
