// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#define QT_NO_OPENGL
#include <QMessageBox>
#include <QWidget>
#include "citra_qt/sbs_3d_ui.h"
#include "core/settings.h"

// move dialog to centre of left eye in SBS 3D mode
void MoveDialogToLeftEye(QWidget* dialog, QWidget* parent, int offset) {
    if (!Settings::values.toggle_3d) {
        return;
    }
    dialog->adjustSize();
    QRect screen_geometry = parent->geometry();
    screen_geometry.setWidth(screen_geometry.width() / 2);
    int dialog_width = std::min(dialog->width(), screen_geometry.width());
    int dialog_height = std::min(dialog->height(), screen_geometry.height());
    dialog->move(screen_geometry.center().x() + offset - dialog_width / 2,
                 screen_geometry.center().y() - dialog_height / 2);
}

// move dialog to centre of right eye in SBS 3D mode
void MoveDialogToRightEye(QWidget* dialog, QWidget* parent, int offset) {
    if (!Settings::values.toggle_3d) {
        return;
    }
    dialog->adjustSize();
    QRect screen_geometry = parent->geometry();
    screen_geometry.setWidth(screen_geometry.width() / 2);
    int dialog_width = std::min(dialog->width(), screen_geometry.width());
    int dialog_height = std::min(dialog->height(), screen_geometry.height());
    dialog->move(screen_geometry.width() + screen_geometry.center().x() + offset - dialog_width / 2,
                 screen_geometry.center().y() - dialog_height / 2);
}

QMessageBox::StandardButton QMessageBox3D::showNewMessageBox(
    QWidget* parent, QMessageBox::Icon icon, const QString& title, const QString& text,
    QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton) {
    constexpr int parallax = 8;
    QMessageBox left_box(icon, title, text, buttons, parent);
    QMessageBox right_box(icon, title, text, buttons, parent);
    left_box.setDefaultButton(defaultButton);
    right_box.setDefaultButton(defaultButton);
    MoveDialogToLeftEye(&left_box, parent, +parallax);
    MoveDialogToRightEye(&right_box, parent, -parallax);
    right_box.show();
    if (left_box.exec() == -1)
        return QMessageBox::Cancel;
    return left_box.standardButton(left_box.clickedButton());
}

QMessageBox::StandardButton QMessageBox3D::information(QWidget* parent, const QString& title,
                                                       const QString& text, StandardButtons buttons,
                                                       StandardButton defaultButton) {
    return showNewMessageBox(parent, Information, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton QMessageBox3D::question(QWidget* parent, const QString& title,
                                                    const QString& text, StandardButtons buttons,
                                                    StandardButton defaultButton) {
    return showNewMessageBox(parent, Question, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton QMessageBox3D::warning(QWidget* parent, const QString& title,
                                                   const QString& text, StandardButtons buttons,
                                                   StandardButton defaultButton) {
    return showNewMessageBox(parent, Warning, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton QMessageBox3D::critical(QWidget* parent, const QString& title,
                                                    const QString& text, StandardButtons buttons,
                                                    StandardButton defaultButton) {
    return showNewMessageBox(parent, Critical, title, text, buttons, defaultButton);
}

int QMessageBox3D::exec() {
    MoveDialogToLeftEye(this, parentWidget());
    return QMessageBox::exec();
}