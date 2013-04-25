
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
	void setShadowsMode(int m);  
	void setAOMode(int m);  
	void setRaysAO(int r);
	void setRadiusAO(int r);
	void setConeAO(int c);
	void setIntensityAO(int i);
	void setLightDiscretization(int i);
	void setMirrorsMode(int m);
	void setNbMaxReflexion(int m);
	void setRaysPT(int r);
	void setIterationsPT(int i);
	void setPTMode(int m);
	void setfocusBlurSamples(int s);
	void setfocalDistance(double f);
	void setaperture(double f);
	void setDOFMode(int s);
	void setGaussianFilterMode(int m);
    	void setStandardDeviationGauss(int stdDeviation);
    	void setSizeMask(int sizeMask);
private :
	void initControlWidget ();
	void initControlWidget2();

	QActionGroup * actionGroup;
	QGroupBox * controlWidget;
	QGroupBox* controlWidget2;
	QString currentDirectory;

	GLViewer * viewer;
	QLabel* antialiasingLabel;		
	QRadioButton* uniformAA;
	QRadioButton* stochasticAA;
	QLabel* raysAOLabel;
	QLabel* radiusAOLabel;
	QLabel* coneAOLabel;
	QLabel* intensityAOLabel;
	QLabel* softShadowsLabel;
	QRadioButton* softShadowsButton; 
	QLabel* raysPTLabel;
	QLabel* iterPTLabel;
	QRadioButton* ptEnabledButton;
	QLabel* focusBlurSamplesLabel;
	QCheckBox * dofSet;
	QRadioButton* enabledGaussianButton;
	QLabel* standardDeviationGaussLabel;
	QLabel* sizeMaskLabel;

	QImage* image;
	QImage computedImage;
};

#endif // WINDOW_H


// Some Emacs-Hints -- please don't remove:
//
//  Local Variables:
//  mode:C++
//  tab-width:4
//  End:
