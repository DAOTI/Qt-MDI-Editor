#include <QtGui>

#include "editor.h"

Editor::Editor(QWidget *parent)
    : QTextEdit(parent)
{
	/*
	 * 创建一个QAction，用它表示应用程序Window菜单中的编辑器，并且把这个动作与show()和setFocus()槽连接起来。
	 **/
    action = new QAction(this);
    action->setCheckable(true);
    connect(action, SIGNAL(triggered()), this, SLOT(show()));
    connect(action, SIGNAL(triggered()), this, SLOT(setFocus()));

	/*
	 * 由于允许用户创建任意数量的编辑器窗口，所以必须为它们预备一些名字，这样就可以在第一次保存这些窗口之前把它们区分开来。
	 * 处理这种情况的一种常用方式是分配一个包含数字的名称（例如：document1.txt）。
	 **/
	// 使用一个isUntitled变量来区分是用户提供的名称还是程序自动创建的名称。
    isUntitled = true;

    connect(document(), SIGNAL(contentsChanged()),
            this, SLOT(documentWasModified()));

    setWindowIcon(QPixmap(":/images/document.png"));
    setWindowTitle("[*]");
    setAttribute(Qt::WA_DeleteOnClose);
}

/**
 * @brief Editor::newFile
 * newFile()函数会为新的文档产生一个像document1.txt这样的名称。由于当在一个新创建的Editor中调用open()而打开一个已经
 * 存在的文档的时候，我们并不想白白浪费这样一个数字，因而把这段代码放在了newFile()中，而不是放在构造函数中。由于documentNumber
 * 声明为静态变量，所以它可以被所有的Editor实例共享。
 */
void Editor::newFile()
{
    static int documentNumber = 1;

    curFile = tr("document%1.txt").arg(documentNumber);
    setWindowTitle(curFile + "[*]");
    action->setText(curFile);
    isUntitled = true;
    ++documentNumber;
}

/**
 * @brief Editor::save
 * @return 
 * save()函数使用isUntitled变量来决定它是该调用saveFile()，还是saveAs()。
 */
bool Editor::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool Editor::saveAs()
{
    QString fileName =
            QFileDialog::getSaveFileName(this, tr("Save As"), curFile);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

QSize Editor::sizeHint() const
{
    return QSize(72 * fontMetrics().width('x'),
                 25 * fontMetrics().lineSpacing());
}

Editor *Editor::open(QWidget *parent)
{
	// 单开文件选择框，选择合适的文件
    QString fileName =
            QFileDialog::getOpenFileName(parent, tr("Open"), ".");
    if (fileName.isEmpty())// 如果文件不存在，返回一个空指针
        return 0;

	// 如果文件存在，则调用openFile()方法
    return openFile(fileName, parent);
}

Editor *Editor::openFile(const QString &fileName, QWidget *parent)
{
	// 新建一个Editor对象，并完成初始化
    Editor *editor = new Editor(parent);
	// 读入指定的文件
    if (editor->readFile(fileName)) {//读入文件成功，就返回一个Editor
        editor->setCurrentFile(fileName);
        return editor;
    } else {//读入文件失败，用户会得到出现问题的信息
		//删除该编辑器，并返回一个空指针
        delete editor;
        return 0;
    }
}

void Editor::closeEvent(QCloseEvent *event)
{
    if (okToContinue()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void Editor::documentWasModified()
{
    setWindowModified(true);
}

bool Editor::okToContinue()
{
    if (document()->isModified()) {
        int r = QMessageBox::warning(this, tr("MDI Editor"),
                        tr("File %1 has been modified.\n"
                           "Do you want to save your changes?")
                        .arg(strippedName(curFile)),
                        QMessageBox::Yes | QMessageBox::No
                        | QMessageBox::Cancel);
        if (r == QMessageBox::Yes) {
            return save();
        } else if (r == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

bool Editor::saveFile(const QString &fileName)
{
    if (writeFile(fileName)) {
        setCurrentFile(fileName);
        return true;
    } else {
        return false;
    }
}

void Editor::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    isUntitled = false;
    action->setText(strippedName(curFile));
    document()->setModified(false);
    setWindowTitle(strippedName(curFile) + "[*]");
    setWindowModified(false);
}

bool Editor::readFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("MDI Editor"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();
    return true;
}

bool Editor::writeFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("MDI Editor"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << toPlainText();
    QApplication::restoreOverrideCursor();
    return true;
}

QString Editor::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
