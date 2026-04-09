#include "EditorAddKeyStateWindow.h"
#include "ui_EditorAddKeyStateWindow.h"

#include "GuiUtilities.h"

EditorAddKeyStateWindow::EditorAddKeyStateWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditorAddKeyStateWindow)
{
    QWidget::setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);

    ui->setupUi(this);

    // List of key names
    QStringList keyNames = {
        "LBUTTON", "RBUTTON", "CANCEL", "MBUTTON", "BACK", "TAB", "CLEAR", "RETURN",
        "PAUSE", "CAPSLOCK", "ESCAPE", "SPACE", "PAGE UP", "PAGE DOWN", "END", "HOME",
        "LEFT", "UP", "RIGHT", "DOWN", "SELECT", "PRINT SCREEN", "EXECUTE", "SNAPSHOT",
        "INSERT", "DELETE", "HELP", "LSHIFT", "RSHIFT", "LCONTROL", "RCONTROL", "LALT", "RALT",
        "NUMPAD0", "NUMPAD1", "NUMPAD2", "NUMPAD3", "NUMPAD4", "NUMPAD5", "NUMPAD6",
        "NUMPAD7", "NUMPAD8", "NUMPAD9", "MULTIPLY", "ADD", "NUMPADENTER", "SUBTRACT",
        "DECIMAL", "DIVIDE", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
        "F11", "F12", "NUMLOCK", "SCROLLLOCK", "`", "MINUS", "EQUALS", "LBRACKET",
        "RBRACKET", "BACKSLASH", "SEMICOLON", "APOSTROPHE", ",", "PERIOD", "SLASH"
    };

    // Numbers 0-9
    for (char c = '0'; c <= '9'; ++c)
    {
        keyNames << QString(c);
    }

    // Letters A-Z
    for (char c = 'A'; c <= 'Z'; ++c)
    {
        keyNames << QString(c);
    }

    // Insert all keys into the ComboBox
    ui->cbKeys->addItems(keyNames);

    // Disable the native Windows popup menu
    ui->cbKeys->setStyleSheet("QComboBox { combobox-popup: 0; }");

    // Display the vertical scroll bar in the dropdown list only when needed
    ui->cbKeys->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    GuiUtilities::centerWidgetOnScreen(this);

    this->setFixedSize(this->size());
}

EditorAddKeyStateWindow::~EditorAddKeyStateWindow()
{
    delete ui;
}

// Get selected key from ComboBox
QString EditorAddKeyStateWindow::getSelectedKey() const
{
    return ui->cbKeys->currentText();
}

// On pushButton Add clicked
void EditorAddKeyStateWindow::on_pushButtonAdd_clicked()
{
    accept();
}

// On pushButton Cancel clicked
void EditorAddKeyStateWindow::on_pushButtonCancel_clicked()
{
    close();
}
