#include "img_viewer.h"
#include <QApplication>

int main(int argc, char **argv) {
    //  The QApplication class manages the GUI application's control flow and main settings.
    //  For any GUI application using Qt, there is precisely one QApplication object, no matter
    //  whether the application has 0, 1, 2 or more windows at any given time.
  QApplication app(argc, argv);

  app.setOrganizationName("QtProject");
  app.setApplicationName("Image Viewer");

  //  Initialize the image viewer pointer which adds stuff to the GUI
  ImageViewer *imgViewer = new ImageViewer();

  imgViewer->show();

  //  Enters the main event loop and waits until exit() is called, then returns the value that was set to exit()
  return app.exec();
}
