#include <QtGui>

#include "editor.h"
#include "mainwindow.h"

MainWindow::MainWindow()
{
    mdiArea = new QMdiArea;//创建一个QMdiArea对象
    setCentralWidget(mdiArea);//成为中央窗口部件
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(updateActions()));//建立与window菜单的槽连接

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    setWindowIcon(QPixmap(":/images/icon.png"));
    setWindowTitle(tr("MDI Editor"));
	/**
	 * @brief QTimer::singleShot
	 * 把单触发定时器的时间间隔设置为0毫秒,以调用loadFiles()函数。对于这样的定时器，只要事件循环一空闲就会触发它。
	 * 实际上，这意味着只要构造函数结束，同时在主窗口显示出来之后，它就会调用loadFiles()。如果不这样做，而且如果还
	 * 需要加载许多大文件的话，那么该构造函数在文件加载完毕之前就无法结束。在此期间，用户极有可能在屏幕上看不见任何
	 * 东西，这样他可能会认为是应用程序启动失败了。
	 */
    QTimer::singleShot(0, this, SLOT(loadFiles()));//把单触发定时器的时间间隔设置为0毫秒
}

void MainWindow::loadFiles()
{
	/**
	 * @brief args
	 * QApplication::arguments():获取命令行中的参数
	 */
    QStringList args = QApplication::arguments();//获取命令行的参数
    args.removeFirst();
    if (!args.isEmpty()) {
        foreach (QString arg, args)
            openFile(arg);//文件不为NULL的时候，打开文件
        mdiArea->cascadeSubWindows();
    } else {
        newFile();//新建文件
    }
    mdiArea->activateNextSubWindow();//对编辑器窗口赋予焦点
}

void MainWindow::newFile()
{
    Editor *editor = new Editor;//创建一个Editor窗口部件，并且把它传递给addEditor()私有函数
    editor->newFile();
    addEditor(editor);
}

void MainWindow::openFile(const QString &fileName)
{
    Editor *editor = Editor::openFile(fileName, this);
    if (editor)
        addEditor(editor);
}

/**
 * @brief MainWindow::closeEvent
 * @param event
 * 重新实现closeEvent()函数来关闭所有的子窗口，从而使得每个子窗口都要接收一个关闭事件。
 * 如果没有在MainWindow中重新实现它的closeEvent()，那么将不会给用户留下那些“未保存变化”
 * 进行存储的任何机会。
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    mdiArea->closeAllSubWindows();
    if (!mdiArea->subWindowList().isEmpty()) {//如果子窗口之一“忽略”了它的关闭事件，那么也就忽略了对MainWindow的关闭事件。
        event->ignore();
    } else {//否则，我们就接收它，结果就是Qt会关闭整个应用程序。
        event->accept();
    }
}

void MainWindow::open()//该函数会弹出一个文件对话框
{
    Editor *editor = Editor::open(this);
    if (editor)
        addEditor(editor);
}

void MainWindow::save()
{
    if (activeEditor())
        activeEditor()->save();
}

void MainWindow::saveAs()
{
    if (activeEditor())
        activeEditor()->saveAs();
}

void MainWindow::cut()
{
    if (activeEditor())
        activeEditor()->cut();
}

void MainWindow::copy()
{
    if (activeEditor())
        activeEditor()->copy();
}

void MainWindow::paste()
{
    if (activeEditor())
        activeEditor()->paste();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About MDI Editor"),
            tr("<h2>Editor 1.1</h2>"
               "<p>Copyright &copy; 2007 Software Inc."
               "<p>MDI Editor is a small application that demonstrates "
               "QMdiArea."));
}

void MainWindow::updateActions()
{
    bool hasEditor = (activeEditor() != 0);
    bool hasSelection = activeEditor()
                        && activeEditor()->textCursor().hasSelection();

    saveAction->setEnabled(hasEditor);
    saveAsAction->setEnabled(hasEditor);
    cutAction->setEnabled(hasSelection);
    copyAction->setEnabled(hasSelection);
    pasteAction->setEnabled(hasEditor);
    closeAction->setEnabled(hasEditor);
    closeAllAction->setEnabled(hasEditor);
    tileAction->setEnabled(hasEditor);
    cascadeAction->setEnabled(hasEditor);
    nextAction->setEnabled(hasEditor);
    previousAction->setEnabled(hasEditor);
    separatorAction->setVisible(hasEditor);

    if (activeEditor())
		/*
		 * 在QAction上调用setChecked(),表示这是一个激活窗口。
		 */
        activeEditor()->windowMenuAction()->setChecked(true);
}

void MainWindow::createActions()
{
    newAction = new QAction(tr("&New"), this);
    newAction->setIcon(QIcon(":/images/new.png"));
    newAction->setShortcut(QKeySequence::New);
    newAction->setStatusTip(tr("Create a new file"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

    openAction = new QAction(tr("&Open..."), this);
    openAction->setIcon(QIcon(":/images/open.png"));
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

    saveAction = new QAction(tr("&Save"), this);
    saveAction->setIcon(QIcon(":/images/save.png"));
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save the file to disk"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAction = new QAction(tr("Save &As..."), this);
    saveAsAction->setStatusTip(tr("Save the file under a new name"));
    connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    cutAction = new QAction(tr("Cu&t"), this);
    cutAction->setIcon(QIcon(":/images/cut.png"));
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setStatusTip(tr("Cut the current selection to the "
                               "clipboard"));
    connect(cutAction, SIGNAL(triggered()), this, SLOT(cut()));

    copyAction = new QAction(tr("&Copy"), this);
    copyAction->setIcon(QIcon(":/images/copy.png"));
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setStatusTip(tr("Copy the current selection to the "
                                "clipboard"));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setIcon(QIcon(":/images/paste.png"));
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setStatusTip(tr("Paste the clipboard's contents at "
                                 "the cursor position"));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));

    closeAction = new QAction(tr("Cl&ose"), this);
    closeAction->setShortcut(QKeySequence::Close);
    closeAction->setStatusTip(tr("Close the active window"));
    connect(closeAction, SIGNAL(triggered()),
            mdiArea, SLOT(closeActiveSubWindow()));

    closeAllAction = new QAction(tr("Close &All"), this);
    closeAllAction->setStatusTip(tr("Close all the windows"));
    connect(closeAllAction, SIGNAL(triggered()), this, SLOT(close()));

    tileAction = new QAction(tr("&Tile"), this);
    tileAction->setStatusTip(tr("Tile the windows"));
    connect(tileAction, SIGNAL(triggered()),
            mdiArea, SLOT(tileSubWindows()));

    cascadeAction = new QAction(tr("&Cascade"), this);
    cascadeAction->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAction, SIGNAL(triggered()),
            mdiArea, SLOT(cascadeSubWindows()));

    nextAction = new QAction(tr("Ne&xt"), this);
    nextAction->setShortcut(QKeySequence::NextChild);
    nextAction->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAction, SIGNAL(triggered()),
            mdiArea, SLOT(activateNextSubWindow()));

    previousAction = new QAction(tr("Pre&vious"), this);
    previousAction->setShortcut(QKeySequence::PreviousChild);
    previousAction->setStatusTip(tr("Move the focus to the previous "
                                    "window"));
    connect(previousAction, SIGNAL(triggered()),
            mdiArea, SLOT(activatePreviousSubWindow()));

    separatorAction = new QAction(this);
    separatorAction->setSeparator(true);

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAction = new QAction(tr("About &Qt"), this);
    aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    windowActionGroup = new QActionGroup(this);
}

/**
 * @brief MainWindow::createMenus
 * createMenus()私有函数会让这些动作和window菜单一起配合工作。
 */
void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAction);
    editMenu->addAction(copyAction);
    editMenu->addAction(pasteAction);

    windowMenu = menuBar()->addMenu(tr("&Window"));
    windowMenu->addAction(closeAction);
    windowMenu->addAction(closeAllAction);
    windowMenu->addSeparator();
    windowMenu->addAction(tileAction);
    windowMenu->addAction(cascadeAction);
    windowMenu->addSeparator();
    windowMenu->addAction(nextAction);
    windowMenu->addAction(previousAction);
    windowMenu->addAction(separatorAction);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAction);
    fileToolBar->addAction(openAction);
    fileToolBar->addAction(saveAction);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAction);
    editToolBar->addAction(copyAction);
    editToolBar->addAction(pasteAction);
}

void MainWindow::createStatusBar()
{
    readyLabel = new QLabel(tr(" Ready"));
    statusBar()->addWidget(readyLabel, 1);
}

/**
 * @brief MainWindow::addEditor
 * @param editor
 * addEditor()私有函数会从newFile()和open()中得到调用，以完成一个新的Editor窗口部件初始化。
 * 它以创建两个信号——槽的连接开始。根据是否有选中的任意文本，这两个连接可以确保Edit->Cut和Edit->Copy
 * 菜单的启用或者禁用。
 */
void MainWindow::addEditor(Editor *editor)
{
    connect(editor, SIGNAL(copyAvailable(bool)),
            cutAction, SLOT(setEnabled(bool)));
    connect(editor, SIGNAL(copyAvailable(bool)),
            copyAction, SLOT(setEnabled(bool)));

	/**
	 * @brief subWindow
	 * QMdiArea::addSubWindow()函数创建一个新的QMdiSubWindow，把作为参数传递的该窗口部件放进子窗口
	 * 中，并且返回还子窗口。
	 */
    QMdiSubWindow *subWindow = mdiArea->addSubWindow(editor);
	/*
	 * 创建一个QAction，代表Window菜单中的一个窗口。
	 */
    windowMenu->addAction(editor->windowMenuAction());
    windowActionGroup->addAction(editor->windowMenuAction());
    subWindow->show();
}
/**
 * @brief MainWindow::activeEditor
 * @return 
 * activeEditor()私有函数返回一个类型为Editor的指针，它指向当前激活的子窗口；或者在没有激活的子窗口时，返回一个空指针。
 */
Editor *MainWindow::activeEditor()
{
    QMdiSubWindow *subWindow = mdiArea->activeSubWindow();
    if (subWindow)
        return qobject_cast<Editor *>(subWindow->widget());
    return 0;
}
