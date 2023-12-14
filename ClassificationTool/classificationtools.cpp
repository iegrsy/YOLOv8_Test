#include "classificationtools.h"
#include "ui_classificationtools.h"

#include <QDialog>
#include <QPushButton>
#include <QDebug>

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

#include <QFileDialog>
#include <QImageReader>
#include <QWidget>
#include <QKeyEvent>
#include <QKeySequence>
#include <QShortcut>

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
		this->currentIndex = 0;
		this->activeFolderInfoList = &this->folder1InfoList;
		ui->frame_1->setStyleSheet("QFrame#frame_1 {border: 1px solid #0000FF}");
		ui->frame_2->setStyleSheet("");
	 });
	 QObject::connect(ui->folder2ActionBtn, &QPushButton::clicked, this, [&](){
		this->currentIndex = 0;
		this->activeFolderInfoList = &this->folder2InfoList;
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
	 QObject::connect(ui->previousBtn, &QPushButton::pressed, this, &ClassificationTools::onNextPressed);
	 QObject::connect(ui->moveBtn, &QPushButton::pressed, this, &ClassificationTools::onNextPressed);
	 QObject::connect(ui->undoBtn, &QPushButton::pressed, this, &ClassificationTools::onUndoPressed);
}

ClassificationTools::~ClassificationTools()
{
	delete ui;
}

QString workingPath = "/home/mert/Documents/person_filtered/person"; // QDir::homePath()

void ClassificationTools::onClickSelectFolder1()
{
	QString dirName = QFileDialog::getExistingDirectory(this, "Open a folder", workingPath);
	 ui->filePath1->setText(dirName);
	this->folder1InfoList = QDir(dirName).entryInfoList(QDir::Files);
}

void ClassificationTools::onClickSelectFolder2()
{
	QString dirName = QFileDialog::getExistingDirectory(this, "Open a folder", workingPath);
	 ui->filePath2->setText(dirName);
	this->folder2InfoList = QDir(dirName).entryInfoList(QDir::Files);
}

void ClassificationTools::listFolderPaths1(QString path)
{
	listImages(path);
}

void ClassificationTools::listFolderPaths2(QString path)
{
	listImages(path);
}

void ClassificationTools::listImages(QString path) {
	qDebug() << path;
}

void ClassificationTools::onNextPressed() {
	if (this->activeFolderInfoList == nullptr) {
		qDebug() << "Not set current folder !!!!!!";
		return;
	}

	auto it = this->activeFolderInfoList;
	this->currentIndex++;
	if (this->currentIndex > it->size() - 1)
		this->currentIndex = it->size() - 1;

	auto fi = it->at(this->currentIndex);
	qDebug() << "next" << fi.fileName();
	ui->previewImage->setPixmap(QPixmap(fi.absoluteFilePath()));
}

void ClassificationTools::onPreviousPressed() {
	if (this->activeFolderInfoList == nullptr) {
		qDebug() << "Not set current folder !!!!!!";
		return;
	}

	auto it = this->activeFolderInfoList;
	this->currentIndex--;
	if (this->currentIndex < 0)
		this->currentIndex = 0;

	auto fi = it->at(this->currentIndex);
	qDebug() << "pre" << fi.fileName();
	ui->previewImage->setPixmap(QPixmap(fi.absoluteFilePath()));
}

void ClassificationTools::onMovePressed() {
	qDebug() << "move";
}

void ClassificationTools::onUndoPressed() {
	qDebug() << "undo";
}

