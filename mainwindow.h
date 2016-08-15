#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <QMainWindow>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QStandardPaths>
#include <QGraphicsPixmapItem>
#include <QStringList>
#include <math.h>

#include "extcolordefs.h"

namespace Ui {
class MainWindow;
}

typedef double decimal_t;

#define decimalDiv(a,b) ((decimal_t)(((decimal_t)(a))/((decimal_t)(b))))

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QFileDialog *dialog;
    QImage *image;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *pixmapItem;
    int originalImageWidth,originalImageHeight;
    uint32_t *originalImageData;
    uint32_t *currentNonRotatedImageData;
    int currentDegs;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static uint32_t *qImageToBitmapData(QImage *image);
    static decimal_t bilinearInterpolate(decimal_t c00, decimal_t c10, decimal_t c01, decimal_t c11, decimal_t w1, decimal_t w2, decimal_t w3, decimal_t w4);

public slots:
    void browseBtnClicked();
    void loadBtnClicked();
    void fitToWindow();
    void resetZoom();
    void dialogFileSelected(QString path);
    void rotateBtnClicked();
    void rotate45DegLeftBtnClicked();
    void rotate45DegRightBtnClicked();
    void flipVerticallyBtnClicked();
    void flipHorizontallyBtnClicked();
    void resetBtnClicked();
    void rotateImage();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
