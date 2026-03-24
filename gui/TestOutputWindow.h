#ifndef TESTOUTPUTWINDOW_H
#define TESTOUTPUTWINDOW_H

#include <QDialog>
#include <QFile>
#include <QIntValidator>

#include "../Global.h"

namespace Ui {
class TestOutputWindow;
}

class TestOutputWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TestOutputWindow(QWidget *parent = nullptr);
    ~TestOutputWindow();

    // Get command
    FunctionCommand getCommand() const;

signals:
    // Signal to send the command to the OutputHookerCore
    void sendTestCommand(const FunctionCommand &cmd);

private slots:
    // Update command text in lineEditCommand
    void updateCommandDisplay();

    // Clear error style
    void clearErrorStyle();

    // On pushButton Test clicked
    void on_pushButtonTest_clicked();

    // On pushButton Cancel clicked
    void on_pushButtonCancel_clicked();

private:
    Ui::TestOutputWindow *ui;

    // Handle function ComboBox
    void handleFunctionChanged(int index);

    // Setup functions in UI
    void setupComPortUI(CommandType cmd);
    void setupGun4irUI(CommandType cmd);
    void setupUltimarcUI(CommandType cmd);
    void setupAudioUI();

    // Check test command
    bool checkCommand(const FunctionCommand &cmd);

    // Validate different values for checkCommand
    bool validateComPortValues(const FunctionCommand &cmd);
    bool validateLedIntensityValue(const FunctionCommand &cmd);
    bool validateWavAudioFile(const FunctionCommand &cmd);

    // Set parameter Label visibility
    void setParamLabelVisibility(bool l1, bool l2, bool l3);

    // Set parameter ComboBox visibiity
    void setParamComboBoxVisibility(bool cb1, bool cb2, bool cb3);

    // Set parameter LineEdit visibiity
    void setParamLineEditVisibility(bool le1, bool le2, bool le3);

    // Reset all inputs
    void resetInputs();

    // Validation 0-255
    QIntValidator* validator255;
};

#endif // TESTOUTPUTWINDOW_H
