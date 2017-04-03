#ifndef IMG_VIEWER_H
#define IMG_VIEWER_H

#include <QMainWindow>
#include <QCheckBox>
#include <QLabel>
#include <QPixmap>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QPlainTextEdit>
class QDateTimeEdit;
class QSpinBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QCheckBox;

// ":" is just like "extends" in Java
class ImageViewer : public QMainWindow {
    Q_OBJECT // no ; required.  Put this inside any Qt GUI class

public:

    //    Initialize main window
    explicit ImageViewer(QWidget *parent = 0);

    //    Destructor
    virtual ~ImageViewer();

    // Camera parameters
    float left, right, top, bottom, ne, fa, eye_x, eye_y, eye_z, center_x, center_y, center_z, up_x, up_y, up_z;

    // Data to keep track of image processing parameters
    float angle, sigma;
    int radius;
    int applyProc[9] = {0,0,0,0,0,0,0,0,0};

private slots:

    // Slot for the menus and the text box
    void openCam();
    void openObj();
    bool save();
    bool saveAs();

    // Slot to rasterize and apply the image processing code.
    void rasterize();

    // Slots for the radio buttons
    void setdef();
    void setflat();
    void setwhit();
    void setgour();
    void setgour_z();
    void setbary();
    void setbary_z();

    //Slots for the check boxes
    void gray_im(int state);
    void flip_im(int state);
    void flop_im(int state);
    void trans_im(int state);
    void box_im(int state);
    void med_im(int state);
    void gauss_im(int state);
    void rot_im(int state);
    void sob_im(int state);

    // Update the spin box when camera opened. Slot also connected to the open file menu option
    void updateParams();

private:
    //  QLabel is typically used for displaying text, but it can also display an image.
    QLabel *imgLabel;

    // Variable to store the image
    QPixmap *img;

    //  Functions for text boxes and menus
    void createActions();
    void createMenus();
    void createStatusBar();
    void setSpinValues();
    void getSpinValues();
    void loadCamFile(const QString &fileName);
    void loadObjFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentCamFile(const QString &fileName);
    void setCurrentObjFile(const QString &fileName);
    void setCurrentOutFile(const QString &fileName);
    void setCurrentOpt(const QString &opt);
    QString strippedName(const QString &fullFileName);

    //  For the text boxes
    QPlainTextEdit *camFile;
    QPlainTextEdit *objFile;
    QString curCam;
    QString curObj;

    // Store the current output file and option
    QString curOut;
    QString curOpt;

    // Actions for text box and file menus
    QMenu *fileMenu;
    QToolBar *fileToolBar;
    QAction *openCamAct;
    QAction *openObjAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;

    // Create spin box for parameters
    void createDoubleSpinBoxes();
    QGroupBox *doubleSpinBoxesGroup;
    QDoubleSpinBox *groupSeparatorSpinBox_d;

    // Doubles spin box to hold the parameters
    QDoubleSpinBox *leftSpinBox,*rightSpinBox,*topSpinBox,*bottomSpinBox,*nearSpinBox,*farSpinBox,*eye_xSpinBox,
    *eye_ySpinBox,*eye_zSpinBox,*center_xSpinBox,*center_ySpinBox,*center_zSpinBox,*up_xSpinBox,*up_ySpinBox,
    *up_zSpinBox;
    QPushButton *RastButton;

    // Create box group for shading options
    void createRadioGroup();
    QGroupBox *RadioGroup;
    QRadioButton *def, *whit, *flat, *gour, *gour_z, *bary, *bary_z;

    //Image processing options
    void createProcGroup();
    QGroupBox *ProcGroup;
    QCheckBox *gray, *flip, *flop, *trans, *box, *med, *gauss, *rot, *sobel;
    QSpinBox *win_size;
    QDoubleSpinBox *sig, *ang;


};

#endif /* IMG_VIEWER_H */
