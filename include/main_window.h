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
#include <QMenu>
#include <QAction>
#include <QMimeData>
#include <QGroupBox>
#include <QSlider>
#include <QVBoxLayout>
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
    void showAboutQt();
    
    // Custom parameter slots
    void threshold_slider_changed(int value);
    void coef_r_slider_changed(int value);
    void coef_g_slider_changed(int value);
    void coef_b_slider_changed(int value);
    
private:
    void setup_ui();
    void setup_connections();
    void switch_language(const QString& lang);
    void retranslate_ui();
    
    // Custom parameter functions
    void setup_custom_param_ui(QVBoxLayout* main_layout);
    void update_custom_param_visibility();
    void save_custom_params();
    void load_custom_params();
    
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
    
    // Menu
    QMenu* helpMenu;
    QAction* aboutQtAction;
    
    // Custom parameter UI elements
    QGroupBox* custom_params_group;
    QSlider* threshold_slider;
    QSlider* coef_r_slider;
    QSlider* coef_g_slider;
    QSlider* coef_b_slider;
    QLabel* threshold_value_label;
    QLabel* coef_r_value_label;
    QLabel* coef_g_value_label;
    QLabel* coef_b_value_label;

    // State
    QString input_folder;
    QString input_file;  // Single file input mode
    QString output_folder;
    fbiu::ProcessFunction current_function;
    bool is_single_file_mode = false;
    fbiu::CustomLumaParams current_custom_params;
    
    // Translation
    QTranslator translator;
    QString current_language;
};

} // namespace fbiu
