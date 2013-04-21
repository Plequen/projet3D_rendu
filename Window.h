#ifndef WINDOW_H
#define WINDOW_H

#include "GLViewer.h"
#include <QMainWindow>
#include <QAction>
#include <QToolBar>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QComboBox>
#include <QSlider>
#include <QLCDNumber>
#include <QSpinBox>
#include <QImage>
#include <QLabel>

#include <vector>
#include <string>

#include "QTUtils.h"


class Window : public QMainWindow {
    Q_OBJECT
public:
    Window();
    virtual ~Window();

    static void showStatusMessage (const QString & msg);  
    
public slots :
    void renderRayImage ();
    void setBGColor ();
    void showRayImage ();
    void exportGLImage ();
    void exportRayImage ();
    void about ();
	void setAAMode(int m);
	void setAAGrid(int grid);  
    
private :
    void initControlWidget ();
        
    QActionGroup * actionGroup;
    QGroupBox * controlWidget;
    QString currentDirectory;

    GLViewer * viewer;
	QLabel* antialiasingLabel;		
	QRadioButton* uniformAA;
};

#endif // WINDOW_H


// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
