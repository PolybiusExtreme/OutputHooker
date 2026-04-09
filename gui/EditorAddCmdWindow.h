#ifndef EDITORADDCMDWINDOW_H
#define EDITORADDCMDWINDOW_H

#include <QDialog>
#include <QFile>
#include <QIntValidator>
#include <QAbstractItemView>

#include "../Global.h"

namespace Ui {
class EditorAddCmdWindow;
}

class EditorAddCmdWindow : public QDialog
{
    Q_OBJECT

public:
    explicit EditorAddCmdWindow(QWidget *parent = nullptr);
    ~EditorAddCmdWindow();

    // Get command
    FunctionCommand getCommand() const;

private slots:
    // Update command text in lineEditCommand
    void updateCommandDisplay();

    // Clear error style
    void clearErrorStyle();

    // On pushButton Add clicked
    void on_pushButtonAdd_clicked();

    // On pushButton Cancel clicked
    void on_pushButtonCancel_clicked();

private:
    Ui::EditorAddCmdWindow *ui;

    // Save the last selected function of the function ComboBox
    static int lastSelectedFunction;

    // Handle function ComboBox
    void handleFunctionChanged(int index);

    // Setup functions in UI
    void setupComPortUI(CommandType cmd);
    void setupGun4irUI(CommandType cmd);
    void setupUltimarcUI(CommandType cmd);
    void setupAppUI(CommandType cmd);
    void setupAudioUI();

    // Check command
    bool checkCommand(const FunctionCommand &cmd);

    // Validate different values for checkCommand
    bool validateComPortValues(const FunctionCommand &cmd);
    bool validateLedIntensityValue(const FunctionCommand &cmd);
    bool validateLaunchApp(const FunctionCommand &cmd);
    bool validateCloseApp(const FunctionCommand &cmd);
    bool validateWavAudioFile(const FunctionCommand &cmd);

    // Set parameter Label visibility
    void setParamLabelVisibility(bool l1, bool l2, bool l3, bool l4, bool l5);

    // Set parameter ComboBox visibiity
    void setParamComboBoxVisibility(bool cb1, bool cb2, bool cb3, bool cb4, bool cb5);

    // Set parameter LineEdit visibiity
    void setParamLineEditVisibility(bool le1, bool le2, bool le3, bool le4, bool le5);

    // Reset all inputs
    void resetInputs();

    // Validation 0-255
    QIntValidator* validator255;
};

#endif // EDITORADDCMDWINDOW_H
