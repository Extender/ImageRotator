#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dialog=new QFileDialog(this);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    QStringList filters;
    filters<<"All images (*.jpg *.jpeg *.png *.gif *.bmp)"
           <<"JPEG images (*.jpg *.jpeg)"
           <<"PNG images (*.png)"
           <<"GIF images (*.gif)"
           <<"Bitmaps (*.bmp)";
    dialog->setNameFilters(filters);
    dialog->setDirectory(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    connect(dialog,SIGNAL(fileSelected(QString)),this,SLOT(dialogFileSelected(QString)));
    connect(ui->browseBtn,SIGNAL(clicked(bool)),this,SLOT(browseBtnClicked()));
    connect(ui->loadBtn,SIGNAL(clicked(bool)),this,SLOT(loadBtnClicked()));
    connect(ui->fitToWindowBtn,SIGNAL(clicked(bool)),this,SLOT(fitToWindow()));
    connect(ui->resetZoomBtn,SIGNAL(clicked(bool)),this,SLOT(resetZoom()));
    image=0;
    scene=new QGraphicsScene();
    pixmapItem=new QGraphicsPixmapItem();
    scene->addItem(pixmapItem);
    ui->graphicsView->setScene(scene);
    originalImageData=0;
    currentNonRotatedImageData=0;

    connect(ui->resetBtn,SIGNAL(clicked(bool)),this,SLOT(resetBtnClicked()));
    connect(ui->flipVerticallyBtn,SIGNAL(clicked(bool)),this,SLOT(flipVerticallyBtnClicked()));
    connect(ui->flipHorizontallyBtn,SIGNAL(clicked(bool)),this,SLOT(flipHorizontallyBtnClicked()));
    connect(ui->rotateBtn,SIGNAL(clicked(bool)),this,SLOT(rotateBtnClicked()));
    connect(ui->rotate45DegLeftBtn,SIGNAL(clicked(bool)),this,SLOT(rotate45DegLeftBtnClicked()));
    connect(ui->rotate45DegRightBtn,SIGNAL(clicked(bool)),this,SLOT(rotate45DegRightBtnClicked()));
}

MainWindow::~MainWindow()
{
    free(originalImageData);
    free(currentNonRotatedImageData);
    delete ui;
}

void MainWindow::browseBtnClicked()
{
    dialog->exec();
}

void MainWindow::loadBtnClicked()
{
    QString path=ui->pathBox->text();
    if(path.length()==0)
    {
        QMessageBox::critical(this,"Error","No file selected.");
        return;
    }
    QFile f(path);
    if(!f.exists())
    {
        QMessageBox::critical(this,"Error","The selected file does not exist.");
        return;
    }
    delete image;
    image=new QImage(path);
    if(image->isNull())
    {
        QMessageBox::critical(this,"Error","The selected file has an unsupported format.");
        return;
    }
    currentDegs=0;
    originalImageWidth=image->width();
    originalImageHeight=image->height();
    scene->setSceneRect(0,0,originalImageWidth,originalImageHeight);
    free(originalImageData);
    free(currentNonRotatedImageData);
    originalImageData=qImageToBitmapData(image);
    size_t imageDataSize=originalImageWidth*originalImageHeight*sizeof(uint32_t);
    currentNonRotatedImageData=(uint32_t*)malloc(imageDataSize);
    memcpy(currentNonRotatedImageData,originalImageData,imageDataSize);
    pixmapItem->setPixmap(QPixmap::fromImage(*image));
    ui->graphicsView->viewport()->update();
    fitToWindow();
}

void MainWindow::fitToWindow()
{
    if(image==0||image->isNull())
        return;
    int width=image->width();
    int height=image->height();
    QRect rect=ui->graphicsView->contentsRect();
    int availableWidth=rect.width()-ui->graphicsView->verticalScrollBar()->width();
    int availableHeight=rect.height()-ui->graphicsView->horizontalScrollBar()->height();
    if((width-availableWidth)>(height-availableHeight))
        ui->graphicsView->setZoomFactor((float)((float)availableWidth)/((float)width));
    else if(height>availableHeight)
        ui->graphicsView->setZoomFactor((float)((float)availableHeight)/((float)height));
    else
        ui->graphicsView->setZoomFactor(1.0);
}

void MainWindow::resetZoom()
{
    ui->graphicsView->setZoomFactor(1.0);
}

void MainWindow::dialogFileSelected(QString path)
{
    ui->pathBox->setText(path);
}

void MainWindow::rotateBtnClicked()
{
    // Floating point values may fluctuate; always compare using integers

    int enteredDegValue=ui->degBox->value()%360;
    currentDegs+=enteredDegValue;

    rotateImage();
}

void MainWindow::rotate45DegLeftBtnClicked()
{
    currentDegs-=45;
    rotateImage();
}

void MainWindow::rotate45DegRightBtnClicked()
{
    currentDegs+=45;
    rotateImage();
}

void MainWindow::flipVerticallyBtnClicked()
{
    // Note that this will discard the current rotation

    if(image==0||image->isNull())
        return;

    currentDegs=0;

    uint32_t *newImageData=(uint32_t*)malloc(originalImageWidth*originalImageHeight*sizeof(uint32_t));
    int yPos=originalImageHeight;
    for(int y=0;y<originalImageHeight;y++)
    {
        yPos--;
        int origOffset=yPos*originalImageWidth;
        int offset=y*originalImageWidth;
        for(int x=0;x<originalImageWidth;x++)
        {
            // Do not use originalImageData; use currentNonRotatedImageData.
            newImageData[offset+x]=currentNonRotatedImageData[origOffset+x];
        }
    }
    free(currentNonRotatedImageData);
    currentNonRotatedImageData=newImageData;
    delete image;
    image=new QImage((uchar*)newImageData,originalImageWidth,originalImageHeight,QImage::Format_ARGB32);
    pixmapItem->setPixmap(QPixmap::fromImage(*image));
    scene->setSceneRect(0,0,originalImageWidth,originalImageHeight);
    ui->graphicsView->viewport()->update();
    fitToWindow();
}

void MainWindow::flipHorizontallyBtnClicked()
{
    // Note that this will discard the current rotation

    if(image==0||image->isNull())
        return;

    currentDegs=0;

    uint32_t *newImageData=(uint32_t*)malloc(originalImageWidth*originalImageHeight*sizeof(uint32_t));
    for(int y=0;y<originalImageHeight;y++)
    {
        int offset=y*originalImageWidth;
        int xPos=originalImageWidth;
        for(int x=0;x<originalImageWidth;x++)
        {
            xPos--;
            // Do not use originalImageData; use the current image's data.
            newImageData[offset+x]=currentNonRotatedImageData[offset+xPos];
        }
    }
    free(currentNonRotatedImageData);
    currentNonRotatedImageData=newImageData;
    delete image;
    image=new QImage((uchar*)newImageData,originalImageWidth,originalImageHeight,QImage::Format_ARGB32);
    pixmapItem->setPixmap(QPixmap::fromImage(*image));
    scene->setSceneRect(0,0,originalImageWidth,originalImageHeight);
    ui->graphicsView->viewport()->update();
    fitToWindow();
}

void MainWindow::resetBtnClicked()
{
    if(image==0||image->isNull())
        return;

    currentDegs=0;

    delete image;
    image=new QImage((uchar*)originalImageData,originalImageWidth,originalImageHeight,QImage::Format_ARGB32);
    free(currentNonRotatedImageData);
    size_t imageDataSize=originalImageWidth*originalImageHeight*sizeof(uint32_t);
    currentNonRotatedImageData=(uint32_t*)malloc(imageDataSize);
    memcpy(currentNonRotatedImageData,originalImageData,imageDataSize);
    pixmapItem->setPixmap(QPixmap::fromImage(*image));
    scene->setSceneRect(0,0,originalImageWidth,originalImageHeight);
    ui->graphicsView->viewport()->update();
    fitToWindow();
}


void MainWindow::rotateImage()
{
    if(currentDegs<0)
        currentDegs=360-(abs(currentDegs)%360);
    if(currentDegs>=360)
        currentDegs-=360;

    decimal_t degsToRotate=(((decimal_t)currentDegs)/180.0f)*M_PI;
    uint32_t *newImageData;
    int newImageWidth;
    int newImageHeight;

    if(currentDegs%90==0)
    {
        if(currentDegs==0)
        {
            currentDegs=0;
            // The current image may be rotated; restore currentNonRotatedImageData
            newImageWidth=originalImageWidth;
            newImageHeight=originalImageHeight;
            size_t imageDataSize=newImageWidth*newImageHeight*sizeof(uint32_t);
            newImageData=(uint32_t*)malloc(imageDataSize);
            memcpy(newImageData,currentNonRotatedImageData,imageDataSize);
        }
        else if(currentDegs==90)
        {
            // Flip to right

            newImageWidth=originalImageHeight;
            newImageHeight=originalImageWidth;
            newImageData=(uint32_t*)malloc(newImageWidth*newImageHeight*sizeof(uint32_t));

            for(int y=0;y<newImageHeight;y++)
            {
                int offset=y*newImageWidth;
                int currentX=originalImageHeight;
                for(int x=0;x<newImageWidth;x++)
                {
                    currentX--;
                    newImageData[offset+x]=currentNonRotatedImageData[currentX*originalImageWidth+y];
                }
            }
        }
        else if(currentDegs==180)
        {
            // Not the same as flipping vertically

            newImageWidth=originalImageWidth;
            newImageHeight=originalImageHeight;
            newImageData=(uint32_t*)malloc(newImageWidth*newImageHeight*sizeof(uint32_t));

            int currentY=originalImageHeight;
            for(int y=0;y<newImageHeight;y++)
            {
                currentY--;
                int origOffset=currentY*newImageWidth;
                int offset=y*newImageWidth;
                int currentX=originalImageWidth;
                for(int x=0;x<newImageWidth;x++)
                {
                    currentX--;
                    newImageData[offset+x]=currentNonRotatedImageData[origOffset+currentX];
                }
            }
        }
        else if(currentDegs==270)
        {
            // Flip to left

            newImageWidth=originalImageHeight;
            newImageHeight=originalImageWidth;
            newImageData=(uint32_t*)malloc(newImageWidth*newImageHeight*sizeof(uint32_t));

            int currentY=newImageHeight;
            for(int y=0;y<newImageHeight;y++)
            {
                currentY--;
                int offset=currentY*newImageWidth;
                for(int x=0;x<newImageWidth;x++)
                {
                    newImageData[offset+x]=currentNonRotatedImageData[x*originalImageWidth+y];
                }
            }
        }
    }
    else
    {
        decimal_t rightmostPossibleX=originalImageWidth-1;
        decimal_t bottommostPossibleY=originalImageHeight-1;

        // Both images share the same center

        decimal_t centerX=rightmostPossibleX*0.5;
        decimal_t centerY=bottommostPossibleY*0.5;

        // Calculate new edge points' positions and extract image size from them

        // These edge distances stay the same as the image is rotated

        decimal_t dTopLeftEdge=sqrt(pow2(centerX)+pow2(centerY));
        decimal_t dTopRightEdge=sqrt(pow2(centerX-rightmostPossibleX)+pow2(centerY));
        decimal_t dBottomLeftEdge=sqrt(pow2(centerX)+pow2(centerY-bottommostPossibleY));
        decimal_t dBottomRightEdge=sqrt(pow2(centerX-rightmostPossibleX)+pow2(centerY-bottommostPossibleY));

        // Calculate the new positions of these points (needed to determine the new image's dimensions)

        decimal_t topLeftAngle=atan(decimalDiv(centerY,centerX));
        decimal_t topLeftX=(centerX-cos(topLeftAngle+degsToRotate)*dTopLeftEdge);
        decimal_t topLeftY=(centerY-sin(topLeftAngle+degsToRotate)*dTopLeftEdge);

        decimal_t topRightAngle=atan(decimalDiv(rightmostPossibleX-centerX,centerY));
        decimal_t topRightX=(centerX+sin(topRightAngle+degsToRotate)*dTopRightEdge);
        decimal_t topRightY=(centerY-cos(topRightAngle+degsToRotate)*dTopRightEdge);

        decimal_t bottomLeftAngle=atan(decimalDiv(centerX,bottommostPossibleY-centerY));
        decimal_t bottomLeftX=(centerX-sin(bottomLeftAngle+degsToRotate)*dBottomLeftEdge);
        decimal_t bottomLeftY=(centerY+cos(bottomLeftAngle+degsToRotate)*dBottomLeftEdge);

        decimal_t bottomRightAngle=atan(decimalDiv(bottommostPossibleY-centerY,rightmostPossibleX-centerX));
        decimal_t bottomRightX=(centerX+cos(bottomRightAngle+degsToRotate)*dBottomRightEdge);
        decimal_t bottomRightY=(centerY+sin(bottomRightAngle+degsToRotate)*dBottomRightEdge);

        decimal_t leftmostX=(__min(topLeftX,__min(topRightX,__min(bottomLeftX,bottomRightX))));
        decimal_t topmostY=(__min(topLeftY,__min(topRightY,__min(bottomLeftY,bottomRightY))));
        decimal_t rightmostX=(__max(topLeftX,__max(topRightX,__max(bottomLeftX,bottomRightX))));
        decimal_t bottommostY=(__max(topLeftY,__max(topRightY,__max(bottomLeftY,bottomRightY))));

        // This is calculated correctly:

        newImageWidth=ceil(rightmostX-leftmostX);
        newImageHeight=ceil(bottommostY-topmostY);

        newImageData=(uint32_t*)calloc(newImageWidth*newImageHeight,sizeof(uint32_t));

        int method=ui->methodBox->currentIndex();

        if(method==-1)
            method=0;

        if(method==0) // Nearest neighbor
        {
            for(int y=0;y<newImageHeight;y++)
            {
                decimal_t dY=(decimal_t)y+topmostY;
                int offset=y*newImageWidth;
                for(int x=0;x<newImageWidth;x++)
                {
                    decimal_t dX=(decimal_t)x+leftmostX;
                    decimal_t origX,origY;
                    int rOrigX,rOrigY;

                    // A point's distance to the center remains the same in both images

                    decimal_t distanceToCenter=sqrt(pow2(centerX-dX)+pow2(centerY-dY));
                    decimal_t newAngle;

                    newAngle=atan2(centerY-dY,centerX-dX);
                    origX=(centerX-distanceToCenter*cos(newAngle-degsToRotate));
                    origY=(centerY-distanceToCenter*sin(newAngle-degsToRotate));

                    // Round at the last step

                    rOrigX=round(origX);
                    rOrigY=round(origY);

                    // Check whether point exists

                    if(rOrigX<0||rOrigX>=originalImageWidth||rOrigY<0||rOrigY>=originalImageHeight)
                        continue;

                    newImageData[offset+x]=currentNonRotatedImageData[rOrigY*originalImageWidth+rOrigX];
                }
            }
        }
        else if(method==1) // Bilinear
        {
            int xLim=originalImageWidth-1;
            int yLim=originalImageHeight-1;
            for(int y=0;y<newImageHeight;y++)
            {
                decimal_t dY=(decimal_t)y+topmostY;
                int offset=y*newImageWidth;
                for(int x=0;x<newImageWidth;x++)
                {
                    decimal_t dX=(decimal_t)x+leftmostX;
                    decimal_t origX,origY;
                    int rOrigX,rOrigY; // round
                    int fOrigX,fOrigY; // floor
                    int cOrigX,cOrigY; // ceiling

                    // A point's distance to the center remains the same in both images

                    decimal_t distanceToCenter=sqrt(pow2(centerX-dX)+pow2(centerY-dY));
                    decimal_t newAngle;

                    newAngle=atan2(centerY-dY,centerX-dX);
                    origX=(centerX-distanceToCenter*cos(newAngle-degsToRotate));
                    origY=(centerY-distanceToCenter*sin(newAngle-degsToRotate));

                    // Round at the last step

                    rOrigX=round(origX);
                    rOrigY=round(origY);

                    fOrigX=floor(origX);
                    fOrigY=floor(origY);
                    cOrigX=ceil(origX);
                    cOrigY=ceil(origY);

                    // Check whether point exists

                    if(rOrigX<0||rOrigX>=originalImageWidth||rOrigY<0||rOrigY>=originalImageHeight)
                        continue;

                    const bool checkBounds=fOrigX<=1||cOrigX>=originalImageWidth-2||fOrigY<=1||cOrigY>=originalImageHeight-2;

                    uint32_t c00,c01,c10,c11;

                    if(checkBounds)
                    {
                        c00=currentNonRotatedImageData[fOrigY*originalImageWidth+fOrigX];
                        c10=currentNonRotatedImageData[fOrigY*originalImageWidth+(cOrigX>xLim?fOrigX:cOrigX)];
                        c01=(cOrigY>yLim?c00:currentNonRotatedImageData[(cOrigY)*originalImageWidth+fOrigX]);
                        c11=(cOrigY>yLim?c10:(cOrigX>xLim?currentNonRotatedImageData[(cOrigY)*originalImageWidth+fOrigX]:originalImageData[(cOrigY)*originalImageWidth+(cOrigX)]));
                    }
                    else
                    {
                        c00=currentNonRotatedImageData[fOrigY*originalImageWidth+fOrigX];
                        c10=currentNonRotatedImageData[fOrigY*originalImageWidth+cOrigX];
                        c01=currentNonRotatedImageData[(cOrigY)*originalImageWidth+fOrigX];
                        c11=currentNonRotatedImageData[(cOrigY)*originalImageWidth+(cOrigX)];
                    }

                    decimal_t xDiff=origX-floor(origX);
                    decimal_t xDiffR=1.0-xDiff;
                    decimal_t yDiff=origY-floor(origY);
                    decimal_t yDiffR=1.0-yDiff;

                    decimal_t w1=xDiffR*yDiffR;
                    decimal_t w2=xDiff*yDiffR;
                    decimal_t w3=xDiffR*yDiff;
                    decimal_t w4=xDiff*yDiff;

                    uint32_t newAlpha=bilinearInterpolate(getAlpha(c00),getAlpha(c01),getAlpha(c10),getAlpha(c11),w1,w2,w3,w4);
                    uint32_t newRed=bilinearInterpolate(getRed(c00),getRed(c01),getRed(c10),getRed(c11),w1,w2,w3,w4);
                    uint32_t newGreen=bilinearInterpolate(getGreen(c00),getGreen(c01),getGreen(c10),getGreen(c11),w1,w2,w3,w4);
                    uint32_t newBlue=bilinearInterpolate(getBlue(c00),getBlue(c01),getBlue(c10),getBlue(c11),w1,w2,w3,w4);

                    newImageData[offset+x]=getColor(newAlpha,newRed,newGreen,newBlue);
                }
            }
        }
    }

    delete image;
    image=new QImage((uchar*)newImageData,newImageWidth,newImageHeight,QImage::Format_ARGB32);
    pixmapItem->setPixmap(QPixmap::fromImage(*image));
    scene->setSceneRect(0,0,newImageWidth,newImageHeight);
    ui->graphicsView->viewport()->update();
    fitToWindow();
}

uint32_t *MainWindow::qImageToBitmapData(QImage *image)
{
    int32_t width=image->width();
    int32_t height=image->height();
    uint32_t *out=(uint32_t*)malloc(width*height*sizeof(uint32_t));
    for(int32_t y=0;y<height;y++)
    {
        int32_t offset=y*width;
        QRgb *scanLine=(QRgb*)image->scanLine(y); // Do not free!
        for(int32_t x=0;x<width;x++)
        {
            QRgb color=scanLine[x];
            uint32_t alpha=qAlpha(color);
            uint32_t red=qRed(color);
            uint32_t green=qGreen(color);
            uint32_t blue=qBlue(color);
            out[offset+x]=(alpha<<24)|(red<<16)|(green<<8)|blue;
        }
        // Do not free "scanLine"!
    }
    return out;
}

decimal_t MainWindow::bilinearInterpolate(decimal_t c00, decimal_t c10, decimal_t c01, decimal_t c11, decimal_t w1, decimal_t w2, decimal_t w3, decimal_t w4)
{
    return w1*c00+w2*c01+w3*c10+w4*c11;
}
