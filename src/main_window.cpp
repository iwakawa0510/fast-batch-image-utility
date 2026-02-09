#include "main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QPixmap>
#include <QImage>
#include <QDir>
#include <QCoreApplication>
#include <QPainter>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QMenuBar>
#include <QSettings>
#include <filesystem>

namespace fs = std::filesystem;

namespace fbiu {

MainWindow::MainWindow(QWidget* parent) 
    : QMainWindow(parent), current_function(ProcessFunction::LUMA_TO_ALPHA) {
    setWindowTitle("Fast Batch Image Utility");
    resize(1000, 700);
    
    // Load custom parameters from settings
    load_custom_params();
    
    setup_ui();
    setup_connections();
    
    // Load Japanese by default
    switch_language("ja");
}

void MainWindow::setup_ui() {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    
    QVBoxLayout* main_layout = new QVBoxLayout(central);
    
    // Language selection
    QHBoxLayout* lang_layout = new QHBoxLayout();
    QLabel* lang_label = new QLabel(tr("Language:"), this);
    language_combo = new QComboBox(this);
    language_combo->addItem("日本語", "ja");
    language_combo->addItem("English", "en");
    language_combo->addItem("Français", "fr");
    lang_layout->addWidget(lang_label);
    lang_layout->addWidget(language_combo);
    lang_layout->addStretch();
    main_layout->addLayout(lang_layout);
    
    // Input folder/file
    QHBoxLayout* input_layout = new QHBoxLayout();
    input_button = new QPushButton(tr("Input Folder"), this);
    input_file_button = new QPushButton(tr("Input File"), this);
    input_path_edit = new QLineEdit(this);
    input_path_edit->setPlaceholderText(tr("Select folder or file, or drag & drop"));
    input_layout->addWidget(input_button);
    input_layout->addWidget(input_file_button);
    input_layout->addWidget(input_path_edit);
    main_layout->addLayout(input_layout);
    
    // Output folder
    QHBoxLayout* output_layout = new QHBoxLayout();
    output_button = new QPushButton(tr("Output Folder"), this);
    output_path_edit = new QLineEdit(this);
    output_path_edit->setPlaceholderText(tr("Select output folder"));
    output_layout->addWidget(output_button);
    output_layout->addWidget(output_path_edit);
    main_layout->addLayout(output_layout);
    
    // Function selection
    QHBoxLayout* func_layout = new QHBoxLayout();
    QLabel* func_label = new QLabel(tr("Function:"), this);
    function_combo = new QComboBox(this);
    function_combo->addItem(tr("Luminance → Transparency"), 
                           static_cast<int>(ProcessFunction::LUMA_TO_ALPHA));
    function_combo->addItem(tr("Luminance → Transparency (Custom)"), 
                           static_cast<int>(ProcessFunction::LUMA_TO_ALPHA_CUSTOM));
    function_combo->addItem(tr("Convert to PNG"), 
                           static_cast<int>(ProcessFunction::CONVERT_TO_PNG));
    func_layout->addWidget(func_label);
    func_layout->addWidget(function_combo);
    func_layout->addStretch();
    main_layout->addLayout(func_layout);
    
    // Custom parameters UI (initially hidden)
    setup_custom_param_ui(main_layout);
    
    // Preview section
    QGroupBox* preview_group = new QGroupBox(tr("Preview"), this);
    QVBoxLayout* preview_layout = new QVBoxLayout(preview_group);
    
    QHBoxLayout* preview_mode_layout = new QHBoxLayout();
    QLabel* mode_label = new QLabel(tr("Display Mode:"), this);
    preview_mode_combo = new QComboBox(this);
    preview_mode_combo->addItem(tr("RGBA Normal"));
    preview_mode_combo->addItem(tr("Alpha Only"));
    preview_mode_combo->addItem(tr("Checkerboard"));
    preview_mode_layout->addWidget(mode_label);
    preview_mode_layout->addWidget(preview_mode_combo);
    preview_mode_layout->addStretch();
    preview_layout->addLayout(preview_mode_layout);
    
    QHBoxLayout* preview_images_layout = new QHBoxLayout();
    
    QVBoxLayout* before_layout = new QVBoxLayout();
    QLabel* before_title = new QLabel(tr("Before"), this);
    before_title->setAlignment(Qt::AlignCenter);
    preview_before_label = new QLabel(this);
    preview_before_label->setMinimumSize(400, 300);
    preview_before_label->setAlignment(Qt::AlignCenter);
    preview_before_label->setStyleSheet("QLabel { background-color: #2b2b2b; border: 1px solid #555; }");
    before_layout->addWidget(before_title);
    before_layout->addWidget(preview_before_label);
    
    QVBoxLayout* after_layout = new QVBoxLayout();
    QLabel* after_title = new QLabel(tr("After"), this);
    after_title->setAlignment(Qt::AlignCenter);
    preview_after_label = new QLabel(this);
    preview_after_label->setMinimumSize(400, 300);
    preview_after_label->setAlignment(Qt::AlignCenter);
    preview_after_label->setStyleSheet("QLabel { background-color: #2b2b2b; border: 1px solid #555; }");
    after_layout->addWidget(after_title);
    after_layout->addWidget(preview_after_label);
    
    preview_images_layout->addLayout(before_layout);
    preview_images_layout->addLayout(after_layout);
    preview_layout->addLayout(preview_images_layout);
    
    main_layout->addWidget(preview_group);
    
    // Progress bar
    progress_bar = new QProgressBar(this);
    progress_bar->setValue(0);
    main_layout->addWidget(progress_bar);
    
    // Options
    open_folder_checkbox = new QCheckBox(tr("Open output folder after processing"), this);
    main_layout->addWidget(open_folder_checkbox);
    
    // Execute button
    execute_button = new QPushButton(tr("Execute"), this);
    execute_button->setMinimumHeight(40);
    main_layout->addWidget(execute_button);
    
    // Enable drag & drop
    setAcceptDrops(true);
    input_path_edit->setAcceptDrops(true);
    output_path_edit->setAcceptDrops(true);

    // Help Menu
    helpMenu = menuBar()->addMenu(tr("Help"));
    aboutQtAction = new QAction(tr("About Qt"), this);
    helpMenu->addAction(aboutQtAction);
    
    // Initially hide custom parameters
    update_custom_param_visibility();
}

void MainWindow::setup_custom_param_ui(QVBoxLayout* main_layout) {
    custom_params_group = new QGroupBox(tr("Custom Parameters"), this);
    QVBoxLayout* custom_layout = new QVBoxLayout(custom_params_group);
    
    // Threshold slider
    QHBoxLayout* threshold_layout = new QHBoxLayout();
    QLabel* threshold_label = new QLabel(tr("Threshold:"), this);
    threshold_slider = new QSlider(Qt::Horizontal, this);
    threshold_slider->setRange(0, 255);
    threshold_slider->setValue(current_custom_params.threshold);
    threshold_value_label = new QLabel(QString::number(current_custom_params.threshold), this);
    threshold_value_label->setMinimumWidth(50);
    threshold_layout->addWidget(threshold_label);
    threshold_layout->addWidget(threshold_slider);
    threshold_layout->addWidget(threshold_value_label);
    custom_layout->addLayout(threshold_layout);
    
    // R coefficient slider (0.0-1.0, displayed as 0-1000 for precision)
    QHBoxLayout* coef_r_layout = new QHBoxLayout();
    QLabel* coef_r_label = new QLabel(tr("R Coefficient:"), this);
    coef_r_slider = new QSlider(Qt::Horizontal, this);
    coef_r_slider->setRange(0, 1000);
    coef_r_slider->setValue(static_cast<int>(current_custom_params.coef_r * 1000));
    coef_r_value_label = new QLabel(QString::number(current_custom_params.coef_r, 'f', 3), this);
    coef_r_value_label->setMinimumWidth(50);
    coef_r_layout->addWidget(coef_r_label);
    coef_r_layout->addWidget(coef_r_slider);
    coef_r_layout->addWidget(coef_r_value_label);
    custom_layout->addLayout(coef_r_layout);
    
    // G coefficient slider
    QHBoxLayout* coef_g_layout = new QHBoxLayout();
    QLabel* coef_g_label = new QLabel(tr("G Coefficient:"), this);
    coef_g_slider = new QSlider(Qt::Horizontal, this);
    coef_g_slider->setRange(0, 1000);
    coef_g_slider->setValue(static_cast<int>(current_custom_params.coef_g * 1000));
    coef_g_value_label = new QLabel(QString::number(current_custom_params.coef_g, 'f', 3), this);
    coef_g_value_label->setMinimumWidth(50);
    coef_g_layout->addWidget(coef_g_label);
    coef_g_layout->addWidget(coef_g_slider);
    coef_g_layout->addWidget(coef_g_value_label);
    custom_layout->addLayout(coef_g_layout);
    
    // B coefficient slider
    QHBoxLayout* coef_b_layout = new QHBoxLayout();
    QLabel* coef_b_label = new QLabel(tr("B Coefficient:"), this);
    coef_b_slider = new QSlider(Qt::Horizontal, this);
    coef_b_slider->setRange(0, 1000);
    coef_b_slider->setValue(static_cast<int>(current_custom_params.coef_b * 1000));
    coef_b_value_label = new QLabel(QString::number(current_custom_params.coef_b, 'f', 3), this);
    coef_b_value_label->setMinimumWidth(50);
    coef_b_layout->addWidget(coef_b_label);
    coef_b_layout->addWidget(coef_b_slider);
    coef_b_layout->addWidget(coef_b_value_label);
    custom_layout->addLayout(coef_b_layout);
    
    main_layout->addWidget(custom_params_group);
}

void MainWindow::setup_connections() {
    connect(input_button, &QPushButton::clicked, this, &MainWindow::select_input_folder);
    connect(input_file_button, &QPushButton::clicked, this, &MainWindow::select_input_file);
    connect(output_button, &QPushButton::clicked, this, &MainWindow::select_output_folder);
    connect(execute_button, &QPushButton::clicked, this, &MainWindow::execute_batch);
    connect(input_path_edit, &QLineEdit::textChanged, this, &MainWindow::update_preview);
    connect(output_path_edit, &QLineEdit::textChanged, this, [this]() {
        output_folder = output_path_edit->text();
    });
    connect(function_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::function_changed);
    connect(language_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::language_changed);
    connect(preview_mode_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::update_preview);
    connect(aboutQtAction, &QAction::triggered, this, &MainWindow::showAboutQt);
    
    // Custom parameter slider connections
    connect(threshold_slider, &QSlider::valueChanged, this, &MainWindow::threshold_slider_changed);
    connect(coef_r_slider, &QSlider::valueChanged, this, &MainWindow::coef_r_slider_changed);
    connect(coef_g_slider, &QSlider::valueChanged, this, &MainWindow::coef_g_slider_changed);
    connect(coef_b_slider, &QSlider::valueChanged, this, &MainWindow::coef_b_slider_changed);
}

void MainWindow::select_input_folder() {
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select Input Folder"), input_folder);
    
    if (!dir.isEmpty()) {
        input_folder = dir;
        input_file.clear();
        is_single_file_mode = false;
        input_path_edit->setText(dir);
        update_preview();
    }
}

void MainWindow::select_input_file() {
    QString file = QFileDialog::getOpenFileName(
        this, tr("Select Input File"), input_file,
        tr("Image Files (*.png *.jpg *.jpeg *.tif *.tiff *.tga *.bmp)"));
    
    if (!file.isEmpty()) {
        input_file = file;
        input_folder.clear();
        is_single_file_mode = true;
        input_path_edit->setText(file);
        update_preview();
    }
}

void MainWindow::select_output_folder() {
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select Output Folder"), output_folder);
    
    if (!dir.isEmpty()) {
        output_folder = dir;
        output_path_edit->setText(dir);
    }
}

void MainWindow::execute_batch() {
    if ((input_folder.isEmpty() && input_file.isEmpty()) || output_folder.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), 
            tr("Please select input (folder or file) and output folder"));
        return;
    }
    
    execute_button->setEnabled(false);
    progress_bar->setValue(0);
    
    bool success = false;
    
    if (is_single_file_mode && !input_file.isEmpty()) {
        // Single file processing
        ImageData input_image = ImageProcessor::load_image(input_file.toStdString());
        if (!input_image.is_valid()) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to load input file"));
            execute_button->setEnabled(true);
            return;
        }
        
        ImageData output_image;
        if (current_function == ProcessFunction::LUMA_TO_ALPHA) {
            output_image = ImageProcessor::luma_to_alpha(input_image, 200);
        } else if (current_function == ProcessFunction::LUMA_TO_ALPHA_CUSTOM) {
            output_image = ImageProcessor::luma_to_alpha_custom(input_image, current_custom_params);
        } else {
            output_image = ImageProcessor::process(input_image, current_function);
        }
        
        if (!output_image.is_valid()) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to process image"));
            execute_button->setEnabled(true);
            return;
        }
        
        QFileInfo file_info(input_file);
        QString output_path = QDir(output_folder).filePath(file_info.baseName() + ".png");
        
        success = ImageProcessor::save_png(output_path.toStdString(), output_image);
        progress_bar->setValue(100);
    } else {
        // Batch processing
        ImageProcessor::BatchOptions options;
        options.input_dir = input_folder.toStdString();
        options.output_dir = output_folder.toStdString();
        options.function = current_function;
        options.luma_threshold = 200;  // For standard LUMA_TO_ALPHA
        options.custom_params = current_custom_params;  // For LUMA_TO_ALPHA_CUSTOM
        options.progress_callback = [this](int completed, int total, const std::string& filename) {
            int progress = (completed * 100) / total;
            QMetaObject::invokeMethod(this, [this, progress]() {
                progress_bar->setValue(progress);
            }, Qt::QueuedConnection);
        };
        
        success = ImageProcessor::batch_process(options);
    }
    
    execute_button->setEnabled(true);
    
    if (success) {
        QMessageBox::information(this, tr("Success"), 
            tr("Processing completed successfully"));
        
        if (open_folder_checkbox->isChecked()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(output_folder));
        }
    } else {
        QMessageBox::critical(this, tr("Error"), 
            tr("Processing failed"));
    }
}

void MainWindow::function_changed(int index) {
    current_function = static_cast<ProcessFunction>(
        function_combo->itemData(index).toInt());
    update_custom_param_visibility();
    update_preview();
}

void MainWindow::language_changed(int index) {
    QString lang = language_combo->itemData(index).toString();
    switch_language(lang);
}

void MainWindow::switch_language(const QString& lang) {
    if (current_language == lang) return;
    
    QCoreApplication::removeTranslator(&translator);
    
    // Load translation from application directory
    QString app_dir = QCoreApplication::applicationDirPath();
    QString qm_file = QDir(app_dir).filePath(QString("translations/app_%1.qm").arg(lang));
    
    if (translator.load(qm_file)) {
        QCoreApplication::installTranslator(&translator);
        current_language = lang;
        retranslate_ui();
    }
}

void MainWindow::retranslate_ui() {
    setWindowTitle(tr("Fast Batch Image Utility"));
    input_button->setText(tr("Input Folder"));
    if (input_file_button) {
        input_file_button->setText(tr("Input File"));
    }
    output_button->setText(tr("Output Folder"));
    execute_button->setText(tr("Execute"));
    if (open_folder_checkbox) {
        open_folder_checkbox->setText(tr("Open output folder after processing"));
    }
    
    // Update combo box items
    function_combo->setItemText(0, tr("Luminance → Transparency"));
    function_combo->setItemText(1, tr("Luminance → Transparency (Custom)"));
    function_combo->setItemText(2, tr("Convert to PNG"));
    
    preview_mode_combo->setItemText(0, tr("RGBA Normal"));
    preview_mode_combo->setItemText(1, tr("Alpha Only"));
    preview_mode_combo->setItemText(2, tr("Checkerboard"));

    // Menu
    helpMenu->setTitle(tr("Help"));
    aboutQtAction->setText(tr("About Qt"));
    
    // Custom parameters group
    if (custom_params_group) {
        custom_params_group->setTitle(tr("Custom Parameters"));
    }
}

void MainWindow::update_preview() {
    QString preview_file;
    
    if (is_single_file_mode && !input_file.isEmpty()) {
        preview_file = input_file;
    } else if (!input_folder.isEmpty()) {
        // Find first image in input folder for preview
        QDir dir(input_folder);
        QStringList filters;
        filters << "*.png" << "*.jpg" << "*.jpeg" << "*.tif" << "*.tiff" << "*.tga" << "*.bmp";
        QStringList files = dir.entryList(filters, QDir::Files);
        
        if (files.isEmpty()) {
            preview_before_label->setText(tr("No images found"));
            preview_after_label->setText("");
            return;
        }
        
        preview_file = dir.filePath(files.first());
    } else {
        preview_before_label->setText(tr("No input selected"));
        preview_after_label->setText("");
        return;
    }
    
    if (preview_file.isEmpty()) return;
    
    // Load and display before
    ImageData before = ImageProcessor::load_image(preview_file.toStdString());
    if (!before.is_valid()) return;
    
    QImage qimg_before(before.pixels.data(), before.width, before.height,
                      before.width * before.channels,
                      before.channels == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
    
    QPixmap pix_before = QPixmap::fromImage(qimg_before).scaled(
        preview_before_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    preview_before_label->setPixmap(pix_before);
    
    // Process and display after
    ImageData after;
    if (current_function == ProcessFunction::LUMA_TO_ALPHA_CUSTOM) {
        after = ImageProcessor::luma_to_alpha_custom(before, current_custom_params);
    } else {
        after = ImageProcessor::process(before, current_function);
    }
    
    if (!after.is_valid()) return;
    
    QImage qimg_after(after.pixels.data(), after.width, after.height,
                     after.width * after.channels,
                     after.channels == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
    
    // Apply preview mode
    int preview_mode = preview_mode_combo->currentIndex();
    QPixmap pix_after;
    
    if (preview_mode == 0) {
        // RGBA Normal - just scale the image
        pix_after = QPixmap::fromImage(qimg_after).scaled(
            preview_after_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else if (preview_mode == 1) {
        // Alpha Only - show only alpha channel as grayscale
        QImage alpha_img(qimg_after.size(), QImage::Format_RGB888);
        for (int y = 0; y < qimg_after.height(); ++y) {
            for (int x = 0; x < qimg_after.width(); ++x) {
                QRgb pixel = qimg_after.pixel(x, y);
                uint8_t alpha = qAlpha(pixel);
                alpha_img.setPixel(x, y, qRgb(alpha, alpha, alpha));
            }
        }
        pix_after = QPixmap::fromImage(alpha_img).scaled(
            preview_after_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else if (preview_mode == 2) {
        // Checkerboard - composite with checkerboard pattern
        QSize target_size = preview_after_label->size();
        QPixmap composite(target_size);
        composite.fill(Qt::white);
        
        QPainter painter(&composite);
        
        // Draw checkerboard pattern
        const int checker_size = 20;
        bool dark = false;
        for (int y = 0; y < target_size.height(); y += checker_size) {
            for (int x = 0; x < target_size.width(); x += checker_size) {
                painter.fillRect(x, y, checker_size, checker_size, 
                                dark ? QColor(200, 200, 200) : QColor(255, 255, 255));
                dark = !dark;
            }
            dark = !dark;
        }
        
        // Scale and draw the image on top
        QPixmap scaled_img = QPixmap::fromImage(qimg_after).scaled(
            target_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
        // Center the image
        QPoint offset((target_size.width() - scaled_img.width()) / 2,
                     (target_size.height() - scaled_img.height()) / 2);
        painter.drawPixmap(offset, scaled_img);
        painter.end();
        
        pix_after = composite;
    }
    
    preview_after_label->setPixmap(pix_after);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    const QMimeData* mime_data = event->mimeData();
    if (!mime_data->hasUrls()) {
        return;
    }
    
    QStringList paths;
    for (const QUrl& url : mime_data->urls()) {
        if (url.isLocalFile()) {
            paths.append(url.toLocalFile());
        }
    }
    
    if (!paths.isEmpty()) {
        handle_dropped_paths(paths);
    }
    
    event->acceptProposedAction();
}

void MainWindow::handle_dropped_paths(const QStringList& paths) {
    if (paths.isEmpty()) return;
    
    QString first_path = paths.first();
    QFileInfo file_info(first_path);
    
    if (file_info.isDir()) {
        // Dropped a folder
        input_folder = first_path;
        input_file.clear();
        is_single_file_mode = false;
        input_path_edit->setText(first_path);
        update_preview();
    } else if (file_info.isFile()) {
        // Check if it's an image file
        QString ext = file_info.suffix().toLower();
        QStringList image_exts = {"png", "jpg", "jpeg", "tif", "tiff", "tga", "bmp"};
        
        if (image_exts.contains(ext)) {
            // Dropped a single image file
            input_file = first_path;
            input_folder.clear();
            is_single_file_mode = true;
            input_path_edit->setText(first_path);
            update_preview();
        } else {
            QMessageBox::warning(this, tr("Error"), 
                tr("Dropped file is not a supported image format"));
        }
    }
}

void MainWindow::showAboutQt() {
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::threshold_slider_changed(int value) {
    current_custom_params.threshold = static_cast<uint8_t>(value);
    threshold_value_label->setText(QString::number(value));
    save_custom_params();
    update_preview();
}

void MainWindow::coef_r_slider_changed(int value) {
    current_custom_params.coef_r = value / 1000.0f;
    coef_r_value_label->setText(QString::number(current_custom_params.coef_r, 'f', 3));
    save_custom_params();
    update_preview();
}

void MainWindow::coef_g_slider_changed(int value) {
    current_custom_params.coef_g = value / 1000.0f;
    coef_g_value_label->setText(QString::number(current_custom_params.coef_g, 'f', 3));
    save_custom_params();
    update_preview();
}

void MainWindow::coef_b_slider_changed(int value) {
    current_custom_params.coef_b = value / 1000.0f;
    coef_b_value_label->setText(QString::number(current_custom_params.coef_b, 'f', 3));
    save_custom_params();
    update_preview();
}

void MainWindow::update_custom_param_visibility() {
    if (custom_params_group) {
        custom_params_group->setVisible(current_function == ProcessFunction::LUMA_TO_ALPHA_CUSTOM);
    }
}

void MainWindow::save_custom_params() {
    QSettings settings("Anthropic", "FastBatchImageUtility");
    settings.setValue("custom_threshold", current_custom_params.threshold);
    settings.setValue("custom_coef_r", current_custom_params.coef_r);
    settings.setValue("custom_coef_g", current_custom_params.coef_g);
    settings.setValue("custom_coef_b", current_custom_params.coef_b);
}

void MainWindow::load_custom_params() {
    QSettings settings("Anthropic", "FastBatchImageUtility");
    current_custom_params.threshold = settings.value("custom_threshold", DEFAULT_LUMA_THRESHOLD).toUInt();
    current_custom_params.coef_r = settings.value("custom_coef_r", LUMA_COEF_R).toFloat();
    current_custom_params.coef_g = settings.value("custom_coef_g", LUMA_COEF_G).toFloat();
    current_custom_params.coef_b = settings.value("custom_coef_b", LUMA_COEF_B).toFloat();
}

} // namespace fbiu
