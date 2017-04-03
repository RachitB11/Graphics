#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QtWidgets>
//#include <QSize>
#include "img_viewer.h"
#include "rast_main.h"
#include "raster_tools.h"
#include "img_proc.h"
#include <iostream>

//Define the constructor
ImageViewer::ImageViewer(QWidget *parent) : QMainWindow(parent){

  //  Define a pixmap and a label to put it in
  img = new QPixmap(600,600);
  img->fill(Qt::blue);
  imgLabel = new QLabel(this);
  imgLabel->setPixmap(*img);

  // Setup the text boxes
  camFile = new QPlainTextEdit(tr("Camera File"));
  objFile = new QPlainTextEdit(tr("Object File"));

  // Set up the spin boxes for camera params
  leftSpinBox = new QDoubleSpinBox;
  rightSpinBox = new QDoubleSpinBox;
  topSpinBox = new QDoubleSpinBox;
  bottomSpinBox = new QDoubleSpinBox;
  nearSpinBox = new QDoubleSpinBox;
  farSpinBox = new QDoubleSpinBox;
  eye_xSpinBox = new QDoubleSpinBox;
  eye_ySpinBox = new QDoubleSpinBox;
  eye_zSpinBox = new QDoubleSpinBox;
  center_xSpinBox = new QDoubleSpinBox;
  center_ySpinBox = new QDoubleSpinBox;
  center_zSpinBox = new QDoubleSpinBox;
  up_xSpinBox = new QDoubleSpinBox;
  up_ySpinBox = new QDoubleSpinBox;
  up_zSpinBox = new QDoubleSpinBox;

  // Set up the button to rasterize
  RastButton = new QPushButton("&Rasterize / Re-Rasterize", this);

  // Set up radio buttons for shading options
  def = new QRadioButton(tr("&Default"));
  whit = new QRadioButton(tr("&White"));
  flat = new QRadioButton(tr("&Flat"));
  gour = new QRadioButton(tr("&Gouraud"));
  gour_z = new QRadioButton(tr("&Gouraud_corrected"));
  bary = new QRadioButton(tr("&Barycentric"));
  bary_z = new QRadioButton(tr("&Barycentric_corrected"));

  // Set up check box for image processing options
  gray = new QCheckBox("&Grayscale", this);
  flip = new QCheckBox("&Flip", this);
  flop = new QCheckBox("&Flop", this);
  trans = new QCheckBox("&Transpose", this);
  box = new QCheckBox("&BoxBlur", this);
  med = new QCheckBox("&Median Filter", this);
  gauss = new QCheckBox("&Gaussian Filter", this);
  rot = new QCheckBox("&Rotate", this);
  sobel = new QCheckBox("&Sobel Edge", this);
  win_size = new QSpinBox;
  sig = new QDoubleSpinBox;
  ang = new QDoubleSpinBox;

  // Create the menus, set up the actions and different group boxes
  createActions();
  createMenus();
  createStatusBar();
  createDoubleSpinBoxes();
  createRadioGroup();
  createProcGroup();


  // Setup layout for the entire application and add the widgets
  QVBoxLayout *layout = new QVBoxLayout();
  // setLayout(layout); // doesn't work for main windows
  setCentralWidget(new QWidget());
  centralWidget()->setLayout(layout);

  layout->addWidget(camFile);
  layout->addWidget(objFile);
  layout->addWidget(doubleSpinBoxesGroup);
  layout->addWidget(RadioGroup);

  QHBoxLayout *imlabLayout = new QHBoxLayout();
  imlabLayout->setAlignment(Qt::AlignCenter);
  imlabLayout->addWidget(RastButton);
  imlabLayout->addWidget(imgLabel);
  imlabLayout->addWidget(ProcGroup);
  layout->addLayout(imlabLayout);

  QFont font("times", 8);
  QFontMetrics fm(font);
  int text_h = fm.height();

  camFile->setFixedSize(1000,text_h+20);
  camFile->setFont(font);
  camFile->setReadOnly(true);
  camFile->show();

  objFile->setFixedSize(1000,text_h+20);
  objFile->setFont(font);
  objFile->setReadOnly(true);
  objFile->show();

  doubleSpinBoxesGroup->show();
  RadioGroup->show();

  // Set the cam, obj and outfile to null strings
  setCurrentCamFile("");
  setCurrentObjFile("");
  setCurrentOutFile("");

  setUnifiedTitleAndToolBarOnMac(true);

  // Title for the application window
  setWindowFilePath("Rasterizer");

}

//Define the destructor
ImageViewer::~ImageViewer() {
}

//Define the slots for the file menu actions
void ImageViewer::openCam()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
        loadCamFile(fileName);
}

void ImageViewer::openObj()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
        loadObjFile(fileName);
}

bool ImageViewer::save()
{
    if (curOut.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curOut);
    }
}

bool ImageViewer::saveAs()
{
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList files;
    if (dialog.exec())
        files = dialog.selectedFiles();
    else
        return false;

    return saveFile(files.at(0));
}

//Slot to update the values in the parameters spin boxes
void ImageViewer::updateParams(){
    QFile file(curCam);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(curCam)
                             .arg(file.errorString()));
        return;
    }
    QTextStream in(&file);
    in>>left>>right>>top>>bottom>>ne>>fa>>eye_x>>eye_y>>eye_z>>center_x>>center_y>>center_z>>up_x>>up_y>>up_z;
    setSpinValues();
}

// Slot that rasterizes and performs the image operations, writes the image to the Pixmap and displays to labels
void ImageViewer::rasterize(){
    getSpinValues();
    float params[] = {left,right,top,bottom,ne,fa,eye_x,eye_y,eye_z,center_x,center_y,center_z,up_x,up_y,up_z};
    // Check out QPrintable(QString) as an alternative.
    QByteArray ba = curObj.toLocal8Bit();
    char *ObjDat = ba.data();
    QByteArray ba2 = curOpt.toLocal8Bit();
    char *OptDat = ba2.data();
//    char outputF[] = "temp_output.ppm";
    img_t *rast_img = raster(ObjDat, params, int(img->width()), int(img->height()), OptDat);

    rast_img = process_image(rast_img, applyProc, win_size->value(), ang->value(), sig->value());

    // Convert the img_t format struct to a QImage type
    QImage fin_im((unsigned char *)rast_img->data,rast_img->w,rast_img->h,(rast_img->w)*sizeof(pixel_t),QImage::Format_RGB888);
    // Convert from QImage to QPixmap
    *img = img->fromImage(fin_im);

//    write_ppm(rast_img, outputF);

//    if(!img->load("temp_output.ppm")){
//            qWarning("Failed to load");
//    }


    imgLabel->setPixmap(*img);

    destroy_img(&rast_img);
}

// Slots to update the shading option passed to the rasterizer
void ImageViewer::setdef(){
    setCurrentOpt(QString("--default"));
}

void ImageViewer::setflat(){
    setCurrentOpt(QString("--norm_flat"));
}

void ImageViewer::setwhit(){
    setCurrentOpt(QString("--white"));
}

void ImageViewer::setgour(){
    setCurrentOpt(QString("--norm_gouraud"));
}

void ImageViewer::setgour_z(){
    setCurrentOpt(QString("--norm_gouraud_z"));
}

void ImageViewer::setbary(){
    setCurrentOpt(QString("--norm_bary"));
}

void ImageViewer::setbary_z(){
    setCurrentOpt(QString("--norm_bary_z"));
}

// Slots to check the options for image processing
void ImageViewer::gray_im(int state){
    if (state == Qt::Unchecked) {
        applyProc[0] = 0;
      } else {
        applyProc[0] = 1;
      }
}

void ImageViewer::flip_im(int state){
    if (state == Qt::Unchecked) {
        applyProc[1] = 0;
      } else {
        applyProc[1] = 1;
      }
}

void ImageViewer::flop_im(int state){
    if (state == Qt::Unchecked) {
        applyProc[2] = 0;
      } else {
        applyProc[2] = 1;
      }
}

void ImageViewer::trans_im(int state){
    if (state == Qt::Unchecked) {
        applyProc[3] = 0;
      } else {
        applyProc[3] = 1;
      }
}

void ImageViewer::box_im(int state){
    if (state == Qt::Unchecked) {
        applyProc[4] = 0;
      } else {
        applyProc[4] = 1;
      }
}

void ImageViewer::med_im(int state){
    if (state == Qt::Unchecked) {
        applyProc[5] = 0;
      } else {
        applyProc[5] = 1;
      }
}

void ImageViewer::gauss_im(int state){
    if (state == Qt::Unchecked) {
        applyProc[6] = 0;
      } else {
        applyProc[6] = 1;
      }
}

void ImageViewer::rot_im(int state){
    if (state == Qt::Unchecked) {
        applyProc[7] = 0;
      } else {
        applyProc[7] = 1;
      }
}

void ImageViewer::sob_im(int state){
    if (state == Qt::Unchecked) {
        applyProc[8] = 0;
      } else {
        applyProc[8] = 1;
      }
}

// Method to create the actions and connect the signals and slots of various components of the GUI
void ImageViewer::createActions()
{
    openCamAct = new QAction(tr("&Open Cam"), this);
    openCamAct->setStatusTip(tr("Open an existing Camera file"));
    connect(openCamAct, SIGNAL(triggered()), this, SLOT(openCam()));
    connect(openCamAct, SIGNAL(triggered()), this, SLOT(updateParams()));

    openObjAct = new QAction(tr("&Open Obj"), this);
    openObjAct->setStatusTip(tr("Open an existing Object file"));
    connect(openObjAct, SIGNAL(triggered()), this, SLOT(openObj()));

    saveAct = new QAction(tr("&Save Image"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the Image to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save Image &As"), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the Image under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    connect(def, SIGNAL( clicked() ), this, SLOT(setdef()));
    connect(flat, SIGNAL( clicked() ), this, SLOT(setflat()));
    connect(whit, SIGNAL( clicked() ), this, SLOT(setwhit()));
    connect(gour, SIGNAL( clicked() ), this, SLOT(setgour()));
    connect(gour_z, SIGNAL( clicked() ), this, SLOT(setgour_z()));
    connect(bary, SIGNAL( clicked() ), this, SLOT(setbary()));
    connect(bary_z, SIGNAL( clicked() ), this, SLOT(setbary_z()));

    connect(gray, SIGNAL(stateChanged(int)),this, SLOT(gray_im(int)));
    connect(flip, SIGNAL(stateChanged(int)),this, SLOT(flip_im(int)));
    connect(flop, SIGNAL(stateChanged(int)),this, SLOT(flop_im(int)));
    connect(trans, SIGNAL(stateChanged(int)),this, SLOT(trans_im(int)));
    connect(box, SIGNAL(stateChanged(int)),this, SLOT(box_im(int)));
    connect(med, SIGNAL(stateChanged(int)),this, SLOT(med_im(int)));
    connect(gauss, SIGNAL(stateChanged(int)),this, SLOT(gauss_im(int)));
    connect(rot, SIGNAL(stateChanged(int)),this, SLOT(rot_im(int)));
    connect(sobel, SIGNAL(stateChanged(int)),this, SLOT(sob_im(int)));

    connect(RastButton, SIGNAL (released()),this, SLOT (rasterize()));
}

// Methods to setup the layout of the menu, parameter spin box, shading options group box and image processing
// group boxes
void ImageViewer::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openCamAct);
    fileMenu->addAction(openObjAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
}

void ImageViewer::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void ImageViewer::createDoubleSpinBoxes()
{
    doubleSpinBoxesGroup = new QGroupBox(tr("Camera parameters"));

    QLabel *leftLabel = new QLabel(tr("Left"));
    leftSpinBox->setValue(0.0);
    leftSpinBox->setRange(-99999999, 99999999);
    leftSpinBox->setDecimals(15);

    QLabel *rightLabel = new QLabel(tr("Right"));
    rightSpinBox->setValue(0.0);
    rightSpinBox->setRange(-99999999, 99999999);
    rightSpinBox->setDecimals(15);

    QLabel *topLabel = new QLabel(tr("Top"));
    topSpinBox->setValue(0.0);
    topSpinBox->setRange(-99999999, 99999999);
    topSpinBox->setDecimals(15);

    QLabel *bottomLabel = new QLabel(tr("Bottom"));
    bottomSpinBox->setValue(0.0);
    bottomSpinBox->setRange(-99999999, 99999999);
    bottomSpinBox->setDecimals(15);

    QLabel *nearLabel = new QLabel(tr("Near"));
    nearSpinBox->setValue(0.0);
    nearSpinBox->setRange(-99999999, 99999999);
    nearSpinBox->setDecimals(3);

    QLabel *farLabel = new QLabel(tr("Far"));
    farSpinBox->setValue(0.0);
    farSpinBox->setRange(-99999999, 99999999);
    farSpinBox->setDecimals(3);

    QLabel *eye_xLabel = new QLabel(tr("Eye_x"));
    eye_xSpinBox->setValue(0.0);
    eye_xSpinBox->setRange(-99999999, 99999999);
    eye_xSpinBox->setDecimals(2);

    QLabel *eye_yLabel = new QLabel(tr("Eye_y"));
    eye_ySpinBox->setValue(0.0);
    eye_ySpinBox->setRange(-99999999, 99999999);
    eye_ySpinBox->setDecimals(2);

    QLabel *eye_zLabel = new QLabel(tr("Eye_z"));
    eye_zSpinBox->setValue(0.0);
    eye_zSpinBox->setRange(-99999999, 99999999);
    eye_zSpinBox->setDecimals(2);

    QLabel *center_xLabel = new QLabel(tr("Center_x"));
    center_xSpinBox->setValue(0.0);
    center_xSpinBox->setRange(-99999999, 99999999);
    center_xSpinBox->setDecimals(2);

    QLabel *center_yLabel = new QLabel(tr("Center_y"));
    center_ySpinBox->setValue(0.0);
    center_ySpinBox->setRange(-99999999, 99999999);
    center_ySpinBox->setDecimals(2);

    QLabel *center_zLabel = new QLabel(tr("Center_z"));
    center_zSpinBox->setValue(0.0);
    center_zSpinBox->setRange(-99999999, 99999999);
    center_zSpinBox->setDecimals(2);

    QLabel *up_xLabel = new QLabel(tr("Up_x"));
    up_xSpinBox->setValue(0.0);
    up_xSpinBox->setRange(-99999999, 99999999);
    up_xSpinBox->setDecimals(2);

    QLabel *up_yLabel = new QLabel(tr("Up_y"));
    up_ySpinBox->setValue(0.0);
    up_ySpinBox->setRange(-99999999, 99999999);
    up_ySpinBox->setDecimals(2);

    QLabel *up_zLabel = new QLabel(tr("Up_z"));
    up_zSpinBox->setValue(0.0);
    up_zSpinBox->setRange(-99999999, 99999999);
    up_zSpinBox->setDecimals(2);

    QGridLayout *spinboxlayout = new QGridLayout;
    spinboxlayout->addWidget(leftLabel,0,0);
    spinboxlayout->addWidget(leftSpinBox,1,0);
    spinboxlayout->addWidget(rightLabel,0,1);
    spinboxlayout->addWidget(rightSpinBox,1,1);
    spinboxlayout->addWidget(topLabel,0,2);
    spinboxlayout->addWidget(topSpinBox,1,2);
    spinboxlayout->addWidget(bottomLabel,0,3);
    spinboxlayout->addWidget(bottomSpinBox,1,3);
    spinboxlayout->addWidget(nearLabel,0,4);
    spinboxlayout->addWidget(nearSpinBox,1,4);
    spinboxlayout->addWidget(farLabel,0,5);
    spinboxlayout->addWidget(farSpinBox,1,5);
    spinboxlayout->addWidget(eye_xLabel,2,0);
    spinboxlayout->addWidget(eye_xSpinBox,3,0);
    spinboxlayout->addWidget(eye_yLabel,2,1);
    spinboxlayout->addWidget(eye_ySpinBox,3,1);
    spinboxlayout->addWidget(eye_zLabel,2,2);
    spinboxlayout->addWidget(eye_zSpinBox,3,2);
    spinboxlayout->addWidget(center_xLabel,2,3);
    spinboxlayout->addWidget(center_xSpinBox,3,3);
    spinboxlayout->addWidget(center_yLabel,2,4);
    spinboxlayout->addWidget(center_ySpinBox,3,4);
    spinboxlayout->addWidget(center_zLabel,2,5);
    spinboxlayout->addWidget(center_zSpinBox,3,5);
    spinboxlayout->addWidget(up_xLabel,4,0);
    spinboxlayout->addWidget(up_xSpinBox,5,0);
    spinboxlayout->addWidget(up_yLabel,4,1);
    spinboxlayout->addWidget(up_ySpinBox,5,1);
    spinboxlayout->addWidget(up_zLabel,4,2);
    spinboxlayout->addWidget(up_zSpinBox,5,2);
    doubleSpinBoxesGroup->setLayout(spinboxlayout);
}

void ImageViewer::createRadioGroup(){
    RadioGroup = new QGroupBox(tr("Shading Options"));

    flat->setChecked(true);
    setflat();

    QGridLayout *gbox = new QGridLayout();

    gbox->addWidget(def,0,0);
    gbox->addWidget(flat,0,1);
    gbox->addWidget(whit,0,2);
    gbox->addWidget(gour,0,3);
    gbox->addWidget(gour_z,0,4);
    gbox->addWidget(bary,0,5);
    gbox->addWidget(bary_z,0,6);

    RadioGroup->setLayout(gbox);

}

void ImageViewer::createProcGroup(){
    ProcGroup = new QGroupBox(tr("Processing Tools"));

    QLabel *win_sizeLabel = new QLabel(tr("Radius"));
    win_size->setValue(3);
    win_size->setMaximum(20);

    QLabel *sigLabel = new QLabel(tr("Sigma"));
    sig->setValue(1.4141);
    sig->setRange(-30,30);
    sig->setDecimals(3);

    QLabel *angLabel = new QLabel(tr("Angle"));
    ang->setValue(0.0);
    ang->setRange(-360,360);
    ang->setDecimals(1);

    QGridLayout *gbox = new QGridLayout();
    gbox->addWidget(gray,0,0);
    gbox->addWidget(flip,0,1);
    gbox->addWidget(flop,1,0);
    gbox->addWidget(trans,1,1);
    gbox->addWidget(box,2,0);
    gbox->addWidget(med,2,1);
    gbox->addWidget(gauss,3,0);
    gbox->addWidget(rot,3,1);
    gbox->addWidget(sobel,4,0);
    gbox->addWidget(win_sizeLabel,5,0);
    gbox->addWidget(win_size,5,1);
    gbox->addWidget(sigLabel,6,0);
    gbox->addWidget(sig,6,1);
    gbox->addWidget(angLabel,7,0);
    gbox->addWidget(ang,7,1);

    ProcGroup->setLayout(gbox);

}

// Method to load camera file using the File Dialog
void ImageViewer::loadCamFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    camFile->setPlainText(fileName);
    QApplication::restoreOverrideCursor();
    setCurrentCamFile(fileName);
    statusBar()->showMessage(tr("Cam File loaded"), 2000);
}

// Method to load object file using the File Dialog
void ImageViewer::loadObjFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    objFile->setPlainText(fileName);
    QApplication::restoreOverrideCursor();
    setCurrentObjFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

// Method to save the image file using file dialog
bool ImageViewer::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    img->save(&file, "PNG");

    setCurrentOutFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

// Methods to set current values of the camera file, object file, output file and shading option
void ImageViewer::setCurrentCamFile(const QString &fileName)
{
    curCam = fileName;
}

void ImageViewer::setCurrentObjFile(const QString &fileName)
{
    curObj = fileName;
}

void ImageViewer::setCurrentOutFile(const QString &fileName)
{
    curOut = fileName;
}

void ImageViewer::setCurrentOpt(const QString &opt)
{
    curOpt = opt;
}

// Method to only extract the file name from the entire path
QString ImageViewer::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

// Methods to set and get the spin values
void ImageViewer:: setSpinValues(){
    leftSpinBox->setValue(left);
    rightSpinBox ->setValue(right);
    topSpinBox ->setValue(top);
    bottomSpinBox ->setValue(bottom);
    nearSpinBox ->setValue(ne);
    farSpinBox ->setValue(fa);
    eye_xSpinBox ->setValue(eye_x);
    eye_ySpinBox ->setValue(eye_y);
    eye_zSpinBox ->setValue(eye_z);
    center_xSpinBox ->setValue(center_x);
    center_ySpinBox ->setValue(center_y);
    center_zSpinBox ->setValue(center_z);
    up_xSpinBox ->setValue(up_x);
    up_ySpinBox ->setValue(up_y);
    up_zSpinBox ->setValue(up_z);
}

void ImageViewer:: getSpinValues(){
    left = leftSpinBox->value();
    right =  rightSpinBox->value();
    top = topSpinBox->value();
    bottom = bottomSpinBox->value();
    ne = nearSpinBox->value();
    fa = farSpinBox->value();
    eye_x = eye_xSpinBox->value();
    eye_y = eye_ySpinBox->value();
    eye_z = eye_zSpinBox->value();
    center_x = center_xSpinBox->value();
    center_y = center_ySpinBox->value();
    center_z = center_zSpinBox->value();
    up_x = up_xSpinBox->value();
    up_y = up_ySpinBox->value();
    up_z = up_zSpinBox->value();
}



