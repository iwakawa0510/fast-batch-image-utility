#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QProgressBar>
#include <QTranslator>
#include <QCheckBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include "image_processor.h"

namespace fbiu {

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;
    
protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    
private slots:
    void select_input_folder();
    void select_output_folder();
    void select_input_file();
    void execute_batch();
    void function_changed(int index);
    void language_changed(int index);
    void update_preview();
    void handle_dropped_paths(const QStringList& paths);
    
private:
    void setup_ui();
    void setup_connections();
    void switch_language(const QString& lang);
    void retranslate_ui();
    
    // UI components
    QPushButton* input_button;
    QPushButton* input_file_button;
    QPushButton* output_button;
    QPushButton* execute_button;
    QLineEdit* input_path_edit;
    QLineEdit* output_path_edit;
    QComboBox* function_combo;
    QComboBox* language_combo;
    QProgressBar* progress_bar;
    QCheckBox* open_folder_checkbox;
    
    // Preview
    QLabel* preview_before_label;
    QLabel* preview_after_label;
    QComboBox* preview_mode_combo;
    
    // State
    QString input_folder;
    QString input_file;  // Single file input mode
    QString output_folder;
    fbiu::ProcessFunction current_function;
    bool is_single_file_mode = false;
    
    // Translation
    QTranslator translator;
    QString current_language;
};

} // namespace fbiu
