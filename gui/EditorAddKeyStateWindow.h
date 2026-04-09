#ifndef EDITORADDKEYSTATEWINDOW_H
#define EDITORADDKEYSTATEWINDOW_H

#include <QDialog>
#include <QAbstractItemView>

namespace Ui {
class EditorAddKeyStateWindow;
}

class EditorAddKeyStateWindow : public QDialog
{
    Q_OBJECT

public:
    explicit EditorAddKeyStateWindow(QWidget *parent = nullptr);
    ~EditorAddKeyStateWindow();

    // Get selected key from ComboBox
    QString getSelectedKey() const;

private slots:
    // On pushButton Add clicked
    void on_pushButtonAdd_clicked();

    // On pushButton Cancel clicked
    void on_pushButtonCancel_clicked();

private:
    Ui::EditorAddKeyStateWindow *ui;
};

#endif // EDITORADDKEYSTATEWINDOW_H
