
#include <GL/glew.h>
#include "Window.h"

#include <vector>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <QDockWidget>
#include <QGroupBox>
#include <QButtonGroup>
#include <QMenuBar>
#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QProgressBar>
#include <QCheckBox>
#include <QRadioButton>
#include <QColorDialog>
#include <QLCDNumber>
#include <QPixmap>
#include <QFrame>
#include <QSplitter>
#include <QMenu>
#include <QScrollArea>
#include <QCoreApplication>
#include <QFont>
#include <QSizePolicy>
#include <QImageReader>
#include <QStatusBar>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>

#include "RayTracer.h"

using namespace std;


Window::Window () : QMainWindow (NULL) {
    try {
        viewer = new GLViewer;
    } catch (GLViewer::Exception e) {
        cerr << e.getMessage () << endl;
        exit (1);
    }
    setCentralWidget (viewer);
    
    QDockWidget * controlDockWidget = new QDockWidget (this);
    initControlWidget ();
    controlDockWidget->setWidget (controlWidget);
    controlDockWidget->adjustSize ();
    addDockWidget (Qt::LeftDockWidgetArea, controlDockWidget);
    controlDockWidget->setFeatures (QDockWidget::AllDockWidgetFeatures);

	QDockWidget* controlDockWidget2 = new QDockWidget(this);
	initControlWidget2();
	controlDockWidget2->setWidget(controlWidget2);
	controlDockWidget2->adjustSize();
	addDockWidget(Qt::RightDockWidgetArea, controlDockWidget2);
	controlDockWidget2->setFeatures(QDockWidget::AllDockWidgetFeatures);

    statusBar()->showMessage("");
}

Window::~Window () {

}

void Window::renderRayImage () {
    qglviewer::Camera * cam = viewer->camera ();
    RayTracer * rayTracer = RayTracer::getInstance ();
    qglviewer::Vec p = cam->position ();
    qglviewer::Vec d = cam->viewDirection ();
    qglviewer::Vec u = cam->upVector ();
    qglviewer::Vec r = cam->rightVector ();
    Vec3Df camPos (p[0], p[1], p[2]);
    Vec3Df viewDirection (d[0], d[1], d[2]);
    Vec3Df upVector (u[0], u[1], u[2]);
    Vec3Df rightVector (r[0], r[1], r[2]);
    float fieldOfView = cam->fieldOfView ();
    float aspectRatio = cam->aspectRatio ();
    unsigned int screenWidth = cam->screenWidth ();
    unsigned int screenHeight = cam->screenHeight ();
    QTime timer;
    timer.start ();
	
	const int imagesNumber = 1;
	
	QImage image[imagesNumber];

	for (unsigned int t = 0 ; t < imagesNumber ; t++) {
		Scene* scene = Scene::getInstance();
		// animate objects
		for (unsigned int i = 0 ; i < scene->getObjects().size() ; i++) {
			scene->getObjects()[i].animate(t);
			scene->getObjects()[i].buildKDTree();
		}

		image[t] = rayTracer->render(camPos, viewDirection, upVector, rightVector, fieldOfView, aspectRatio, screenWidth, screenHeight);
	}

	QImage computedImage = image[0];
	/*QRgb* rgb;
	QRgb* rgb2;
	for (int y = 0; y < image[0].height(); y++) {
		rgb2 = (QRgb*)computedImage.scanLine(y); // Il faut que l'image soit en ARGB32 (je pense, voir la doc)
		for (int x = 0; x < image[0].width(); x++) {
			int valueR = 0;
			int valueG = 0;
			int valueB = 0;
			int valueA = 0;
			for (unsigned int t = 1 ; t < imagesNumber ; t++) {
				rgb = (QRgb*)image[t].scanLine(y); // Il faut que l'image soit en ARGB32 (je pense, voir la doc)
				valueR += qRed(rgb[x]); 
				valueG += qGreen(rgb[x]); 
				valueB += qBlue(rgb[x]); 
				valueA += qAlpha(rgb[x]); 
			}
			valueR /= imagesNumber;
			valueG /= imagesNumber;
			valueB /= imagesNumber;
			valueA /= imagesNumber;
			rgb2[x] = qRgba(valueR, valueG, valueB, valueA);	
		}
	}*/

	viewer->setRayImage(computedImage);

    statusBar()->showMessage(QString ("Raytracing performed in ") +
                             QString::number (timer.elapsed ()) +
                             QString ("ms at ") +
                             QString::number (screenWidth) + QString ("x") + QString::number (screenHeight) +
                             QString (" screen resolution"));
    viewer->setDisplayMode (GLViewer::RayDisplayMode);
}

void Window::setAAMode(int m) {
	RayTracer::getInstance()->setAAMode(m);
}

void Window::setAAGrid(int grid) {
	//RayTracer::getInstance()->setAAMode(static_cast<int>(RayTracer::Uniform));
	RayTracer::getInstance()->setAAGrid(grid);
	antialiasingLabel->setText(QString::number(grid));	
	//uniformAA->setChecked(true);	
}

void Window::setShadowsMode(int m) {
	RayTracer::getInstance()->setShadowsMode(m);
}

void Window::setAOMode(int m) {
	RayTracer::getInstance()->setAOMode(m);
}

void Window::setRaysAO(int r) {
	RayTracer::getInstance()->setRaysAO(r);
	raysAOLabel->setText(QString::number(r));
}

void Window::setRadiusAO(int r) {
	float p = (float (r)) / 100.f;
	RayTracer::getInstance()->setPercentageAO(p);
	radiusAOLabel->setText(QString::number(r) + "%");
}

void Window::setConeAO(int c) {
	RayTracer::getInstance()->setConeAO(float (c));
	coneAOLabel->setText(QString::number(c));
}

void Window::setIntensityAO(int i) {
	float intensity = (float (i)) / 20.f;
	RayTracer::getInstance()->setIntensityAO(intensity);
	intensityAOLabel->setText(QString::number(intensity));
}

void Window::setLightDiscretization(int d) {
	RayTracer::getInstance()->setLightDiscretization(d);
	RayTracer::getInstance()->setShadowsMode(static_cast<int>(RayTracer::Soft));
	softShadowsLabel->setText(QString::number(d));
	softShadowsButton->setChecked(true);	
}

void Window::setMirrorsMode(int m) {
	RayTracer::getInstance()->setMirrorsMode(m);
}

void Window::setNbMaxReflexion(int c) {
	RayTracer::getInstance()->setNbMaxReflexion(c);
}

void Window::setRaysPT(int r) {
	RayTracer::getInstance()->setRaysPT(r);
	RayTracer::getInstance()->setPTMode(static_cast<int>(RayTracer::PTEnabled));
	ptEnabledButton->setChecked(true);
	raysPTLabel->setText(QString::number(r));
}

void Window::setIterationsPT(int i) {
	RayTracer::getInstance()->setIterationsPT(i);
	RayTracer::getInstance()->setPTMode(static_cast<int>(RayTracer::PTEnabled));
	ptEnabledButton->setChecked(true);
	iterPTLabel->setText(QString::number(i));
}

void Window::setPTMode(int m) {
	RayTracer::getInstance()->setPTMode(m);
}
void Window::setGaussianFilterMode(int m)
{
	RayTracer::getInstance()->setGaussianFilterMode(m);
}

void Window::setStandardDeviationGauss(int stdDeviation)
{
	//Slider values between 1 and 50
	//Standard Deviation for the filter between 0.1 and 5.0 (divide by 10)
	RayTracer::getInstance()->setGaussianFilterMode(static_cast<int>(RayTracer::GaussianFilterEnabled));
	RayTracer::getInstance()->setStandardDeviationFilter((float) stdDeviation/10);
	enabledGaussianButton->setChecked(true);
	standardDeviationGaussLabel->setText(QString::number((float) stdDeviation/10));
}

void Window::setSizeMask(int sizeMask)
{
	//Slider values between 1 and 10
	//Standard Deviation for the filter between 3 and 21 (odd values only)
	RayTracer::getInstance()->setGaussianFilterMode(static_cast<int>(RayTracer::GaussianFilterEnabled));
	RayTracer::getInstance()->setSizeMask(2*sizeMask+1);
	enabledGaussianButton->setChecked(true);
	sizeMaskLabel->setText(QString::number(2*sizeMask+1));
}
void Window::setfocusBlurSamples( int s) {
	RayTracer::getInstance()->setdofMode(static_cast<int>(RayTracer::DOFEnabled));
	dofSet->setCheckState( Qt::Checked);
	RayTracer::getInstance()->setfocusBlurSamples(s);
	focusBlurSamplesLabel->setText(QString::number(s));
}

void Window::setfocalDistance(double f) {
	RayTracer::getInstance()->setdofMode(static_cast<int>(RayTracer::DOFEnabled));
	dofSet->setCheckState( Qt::Checked);
	RayTracer::getInstance()->setfocalDistance(f);
}
void Window::setaperture(double f) {
	RayTracer::getInstance()->setdofMode(static_cast<int>(RayTracer::DOFEnabled));
	dofSet->setCheckState( Qt::Checked);
	RayTracer::getInstance()->setaperture(f);}

void Window::setDOFMode(int s) {
	RayTracer::getInstance()->setdofMode(s);}


void Window::setBGColor () {
    QColor c = QColorDialog::getColor (QColor (133, 152, 181), this);
    if (c.isValid () == true) {
        cout << c.red () << endl;
        RayTracer::getInstance ()->setBackgroundColor (Vec3Df (c.red (), c.green (), c.blue ()));
        viewer->setBackgroundColor (c);
        viewer->updateGL ();
    }
}

void Window::showRayImage () {
    viewer->setDisplayMode (GLViewer::RayDisplayMode);
}

void Window::exportGLImage () {
    viewer->saveSnapshot (false, false);
}

void Window::exportRayImage () {
    QString filename = QFileDialog::getSaveFileName (this,
                                                     "Save ray-traced image",
                                                     ".",
                                                     "*.jpg *.bmp *.png");
    if (!filename.isNull () && !filename.isEmpty ())
        viewer->getRayImage().save (filename);
}

void Window::about () {
    QMessageBox::about (this, 
                        "About This Program", 
                        "<b>RayMini</b> <br> by <i>Tamy Boubekeur</i>.");
}

void Window::initControlWidget2() {
	controlWidget2 = new QGroupBox ();
	QVBoxLayout* layout = new QVBoxLayout(controlWidget2);

	QGroupBox* rayGroupBox = new QGroupBox("Ray Tracing", controlWidget2);
	QVBoxLayout* rayLayout = new QVBoxLayout(rayGroupBox);
	
	// Antialiasing Settings
	QLabel* aaLabel = new QLabel("Antialiasing", rayGroupBox);
	QButtonGroup* aaButtonGroup = new QButtonGroup(rayGroupBox);
	QRadioButton* noAA = new QRadioButton("None", rayGroupBox);
	uniformAA = new QRadioButton("Uniform", rayGroupBox);
	stochasticAA = new QRadioButton("Stochastic", rayGroupBox);
	aaButtonGroup->addButton(noAA, static_cast<int>(RayTracer::None)); 
	aaButtonGroup->addButton(uniformAA, static_cast<int>(RayTracer::Uniform)); 
	aaButtonGroup->addButton(stochasticAA, static_cast<int>(RayTracer::Stochastic)); 
	noAA->setChecked(true);
	connect(aaButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(setAAMode(int)));
	QSlider* antialiasingSlider = new QSlider(Qt::Horizontal, rayGroupBox);
	antialiasingSlider->setRange(1, 10);
	antialiasingLabel = new QLabel("1", rayGroupBox);
	connect(antialiasingSlider, SIGNAL(valueChanged(int)), this, SLOT(setAAGrid(int)));
	antialiasingSlider->setValue(1);

	QHBoxLayout* uniformAALayout = new QHBoxLayout();
	rayLayout->addWidget(aaLabel);
	rayLayout->addWidget(noAA);
	uniformAALayout->addWidget(uniformAA);
	uniformAALayout->addWidget(antialiasingSlider);
	uniformAALayout->addWidget(antialiasingLabel);
	rayLayout->addLayout(uniformAALayout);
	rayLayout->addWidget(stochasticAA);

	// Shadows settings
	QLabel* shadowsLabel = new QLabel("Shadows", rayGroupBox);
	QButtonGroup* shadowsButtonGroup = new QButtonGroup(rayGroupBox);
	QRadioButton* noShadowsButton = new QRadioButton("None", rayGroupBox);
	QRadioButton* hardShadowsButton = new QRadioButton("Hard", rayGroupBox);
	softShadowsButton = new QRadioButton("Soft", rayGroupBox);
	shadowsButtonGroup->addButton(noShadowsButton, static_cast<int>(RayTracer::NoShadows)); 
	shadowsButtonGroup->addButton(hardShadowsButton, static_cast<int>(RayTracer::Hard)); 
	shadowsButtonGroup->addButton(softShadowsButton, static_cast<int>(RayTracer::Soft)); 
	noShadowsButton->setChecked(true);
	QHBoxLayout* softShadowsLayout = new QHBoxLayout();
	QSlider* softShadowsSlider = new QSlider(Qt::Horizontal, rayGroupBox);
	softShadowsSlider->setRange(0, 200);
	softShadowsSlider->setValue(50);
	softShadowsLabel = new QLabel("50", rayGroupBox);
	softShadowsLayout->addWidget(softShadowsButton);
	softShadowsLayout->addWidget(softShadowsSlider);
	softShadowsLayout->addWidget(softShadowsLabel);
	connect(softShadowsSlider, SIGNAL(valueChanged(int)), this, SLOT(setLightDiscretization(int)));
	connect(shadowsButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(setShadowsMode(int)));

	rayLayout->addWidget(shadowsLabel);
	rayLayout->addWidget(noShadowsButton);
	rayLayout->addWidget(hardShadowsButton);
	rayLayout->addLayout(softShadowsLayout);

	// Ambient Occlusion settings
	QLabel* aoLabel = new QLabel("Ambient Occlusion", rayGroupBox);
	QButtonGroup* aoButtonGroup = new QButtonGroup(rayGroupBox);
	QRadioButton* noAOButton = new QRadioButton("Disabled", rayGroupBox);
	QRadioButton* enabledAOButton = new QRadioButton("Enabled", rayGroupBox);
	QRadioButton* onlyAOButton = new QRadioButton("Only", rayGroupBox);
	aoButtonGroup->addButton(noAOButton, static_cast<int>(RayTracer::AODisabled)); 
	aoButtonGroup->addButton(enabledAOButton, static_cast<int>(RayTracer::AOEnabled)); 
	aoButtonGroup->addButton(onlyAOButton, static_cast<int>(RayTracer::AOOnly)); 
	noAOButton->setChecked(true);
	connect(aoButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(setAOMode(int)));

	QHBoxLayout* raysAOLayout = new QHBoxLayout();
	QLabel* raysAOLabel0 = new QLabel("Rays ", rayGroupBox);
	QSlider* raysAOSlider = new QSlider(Qt::Horizontal, rayGroupBox);
	raysAOSlider->setRange(1, 255);
	raysAOSlider->setValue(10);
	raysAOLabel = new QLabel("10", rayGroupBox);
	connect(raysAOSlider, SIGNAL(valueChanged(int)), this, SLOT(setRaysAO(int)));
	raysAOLayout->addWidget(raysAOLabel0);
	raysAOLayout->addWidget(raysAOSlider);
	raysAOLayout->addWidget(raysAOLabel);

	QHBoxLayout* radiusAOLayout = new QHBoxLayout();
	QLabel* radiusAOLabel0 = new QLabel("Radius ", rayGroupBox);
	QSlider* radiusAOSlider = new QSlider(Qt::Horizontal, rayGroupBox);
	radiusAOSlider->setRange(1, 100);
	radiusAOSlider->setValue(5);
	radiusAOLabel = new QLabel("5%", rayGroupBox);
	connect(radiusAOSlider, SIGNAL(valueChanged(int)), this, SLOT(setRadiusAO(int)));
	radiusAOLayout->addWidget(radiusAOLabel0);
	radiusAOLayout->addWidget(radiusAOSlider);
	radiusAOLayout->addWidget(radiusAOLabel);

	QHBoxLayout* coneAOLayout = new QHBoxLayout();
	QLabel* coneAOLabel0 = new QLabel("Cone ", rayGroupBox);
	QSlider* coneAOSlider = new QSlider(Qt::Horizontal, rayGroupBox);
	coneAOSlider->setRange(0, 180);
	coneAOSlider->setValue(180);
	coneAOLabel = new QLabel("180", rayGroupBox);
	connect(coneAOSlider, SIGNAL(valueChanged(int)), this, SLOT(setConeAO(int)));
	coneAOLayout->addWidget(coneAOLabel0);
	coneAOLayout->addWidget(coneAOSlider);
	coneAOLayout->addWidget(coneAOLabel);

	QHBoxLayout* intensityAOLayout = new QHBoxLayout();
	QLabel* intensityAOLabel0 = new QLabel("Intensity ", rayGroupBox);
	QSlider* intensityAOSlider = new QSlider(Qt::Horizontal, rayGroupBox);
	intensityAOSlider->setRange(0, 20);
	intensityAOSlider->setValue(20);
	intensityAOLabel = new QLabel("1", rayGroupBox);
	connect(intensityAOSlider, SIGNAL(valueChanged(int)), this, SLOT(setIntensityAO(int)));
	intensityAOLayout->addWidget(intensityAOLabel0);
	intensityAOLayout->addWidget(intensityAOSlider);
	intensityAOLayout->addWidget(intensityAOLabel);

	rayLayout->addWidget(aoLabel);
	rayLayout->addWidget(noAOButton);
	rayLayout->addWidget(enabledAOButton);
	rayLayout->addWidget(onlyAOButton);
	rayLayout->addLayout(raysAOLayout);
	rayLayout->addLayout(radiusAOLayout);
	rayLayout->addLayout(coneAOLayout);
	rayLayout->addLayout(intensityAOLayout);

	// mirrors settings
	QHBoxLayout* maxReflexion = new QHBoxLayout();
	QLabel* maxReflexionLabel = new QLabel("Nb Reflexion Max ", rayGroupBox);
	QLabel* mirrorsLabel = new QLabel("Mirrors", rayGroupBox);
	QButtonGroup* mirrorsButtonGroup = new QButtonGroup(rayGroupBox);
	QRadioButton* mirrorsDisabledButton = new QRadioButton("Disabled", rayGroupBox);
	QRadioButton* mirrorsEnabledButton = new QRadioButton("Enabled", rayGroupBox);
	QSlider* mirrorsMaxReflexion = new QSlider(Qt::Horizontal, rayGroupBox);
	mirrorsMaxReflexion->setRange(0, 10);
	mirrorsMaxReflexion->setValue(1);
	mirrorsButtonGroup->addButton(mirrorsDisabledButton, static_cast<int>(RayTracer::MDisabled)); 
	mirrorsButtonGroup->addButton(mirrorsEnabledButton, static_cast<int>(RayTracer::MEnabled)); 
	maxReflexion->addWidget(maxReflexionLabel);
	maxReflexion->addWidget(mirrorsMaxReflexion);
	connect(mirrorsMaxReflexion, SIGNAL(valueChanged(int)), this, SLOT(setNbMaxReflexion(int)));
	mirrorsDisabledButton->setChecked(true);
	connect(mirrorsButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(setMirrorsMode(int)));

	rayLayout->addWidget(mirrorsLabel);
	rayLayout->addWidget(mirrorsDisabledButton);
	rayLayout->addWidget(mirrorsEnabledButton);
	rayLayout->addLayout(maxReflexion);

	// path-tracing settings
	QLabel* ptLabel = new QLabel("Path tracing", rayGroupBox);
	QButtonGroup* ptButtonGroup = new QButtonGroup(rayGroupBox);
	QRadioButton* ptDisabledButton = new QRadioButton("Disabled", rayGroupBox);
	ptEnabledButton = new QRadioButton("Enabled", rayGroupBox);
	ptButtonGroup->addButton(ptDisabledButton, static_cast<int>(RayTracer::PTDisabled)); 
	ptButtonGroup->addButton(ptEnabledButton, static_cast<int>(RayTracer::PTEnabled)); 
	ptDisabledButton->setChecked(true);
	connect(ptButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(setPTMode(int)));

	QHBoxLayout* raysPTLayout = new QHBoxLayout();
	QLabel* raysPTLabel0 = new QLabel("Rays ", rayGroupBox);
	QSlider* raysPTSlider = new QSlider(Qt::Horizontal, rayGroupBox);
	raysPTSlider->setRange(1, 300);
	raysPTSlider->setValue(10);
	raysPTLabel = new QLabel("10", rayGroupBox);
	connect(raysPTSlider, SIGNAL(valueChanged(int)), this, SLOT(setRaysPT(int)));
	raysPTLayout->addWidget(raysPTLabel0);
	raysPTLayout->addWidget(raysPTSlider);
	raysPTLayout->addWidget(raysPTLabel);

	QHBoxLayout* iterPTLayout = new QHBoxLayout();
	QLabel* iterPTLabel0 = new QLabel("Iterations ", rayGroupBox);
	QSlider* iterPTSlider = new QSlider(Qt::Horizontal, rayGroupBox);
	iterPTSlider->setRange(0, 10);
	iterPTSlider->setValue(0);
	iterPTLabel = new QLabel("0", rayGroupBox);
	connect(iterPTSlider, SIGNAL(valueChanged(int)), this, SLOT(setIterationsPT(int)));
	iterPTLayout->addWidget(iterPTLabel0);
	iterPTLayout->addWidget(iterPTSlider);
	iterPTLayout->addWidget(iterPTLabel);

	rayLayout->addWidget(ptLabel);
	rayLayout->addWidget(ptDisabledButton);
	rayLayout->addWidget(ptEnabledButton);
	rayLayout->addLayout(raysPTLayout);
	rayLayout->addLayout(iterPTLayout);

	//Focus Blur settings
	QHBoxLayout* dofLayout = new QHBoxLayout();
	QLabel* dofLabel = new QLabel("Depth of field", rayGroupBox);
	dofSet = new QCheckBox(rayGroupBox);
	dofSet->setCheckState( Qt::Unchecked);
	connect(dofSet, SIGNAL(stateChanged(int)), this, SLOT(setDOFMode(int)));

	QHBoxLayout* focusBlurLayout = new QHBoxLayout();
	QLabel* focusBlurMaxSamples0 = new QLabel("Echantillons ", rayGroupBox);
	QSlider* focusBlurSamplesSlider = new QSlider(Qt::Horizontal, rayGroupBox);
	focusBlurSamplesSlider->setRange(1, 32);
	focusBlurSamplesSlider->setValue(1);
	focusBlurSamplesLabel = new QLabel("1",rayGroupBox);
	connect(focusBlurSamplesSlider, SIGNAL(valueChanged(int)), this, SLOT(setfocusBlurSamples(int)));

	QHBoxLayout* focusDistLayout = new QHBoxLayout();
	QLabel* focusDist0 = new QLabel("Focal Distance ", rayGroupBox);
	QDoubleSpinBox* focusDistSB = new QDoubleSpinBox( rayGroupBox);
	focusDistSB->setValue(5.0);
	connect(focusDistSB, SIGNAL(valueChanged(double)), this, SLOT(setfocalDistance(double)));

	QHBoxLayout* focusAptLayout = new QHBoxLayout();
	QLabel* focusApt0 = new QLabel("Aperture ", rayGroupBox);
	QDoubleSpinBox* focusAptSB = new QDoubleSpinBox(rayGroupBox);
	focusAptSB->setMinimum(1);
	focusAptSB->setValue(5.6);
	connect(focusAptSB, SIGNAL(valueChanged(double)), this, SLOT(setaperture(double)));

	

	dofLayout->addWidget(dofLabel);
	dofLayout->addWidget(dofSet);
	rayLayout->addLayout(dofLayout);

	focusBlurLayout->addWidget(focusBlurMaxSamples0);
	focusBlurLayout->addWidget(focusBlurSamplesSlider);
	focusBlurLayout->addWidget(focusBlurSamplesLabel);
	rayLayout->addLayout(focusBlurLayout);

	focusDistLayout->addWidget(focusDist0);
	focusDistLayout->addWidget(focusDistSB);
	rayLayout->addLayout(focusDistLayout);

	focusAptLayout->addWidget(focusApt0);
	focusAptLayout->addWidget(focusAptSB);
	rayLayout->addLayout(focusAptLayout);






	// other settings
	QPushButton* rayButton = new QPushButton("Render", rayGroupBox);
	rayLayout->addWidget(rayButton);
	connect(rayButton, SIGNAL(clicked()), this, SLOT(renderRayImage()));
	QPushButton* showButton = new QPushButton("Show", rayGroupBox);
	rayLayout->addWidget(showButton);
	connect(showButton, SIGNAL (clicked()), this, SLOT(showRayImage()));
	QPushButton* saveButton  = new QPushButton("Save", rayGroupBox);
	connect(saveButton, SIGNAL(clicked()), this, SLOT(exportRayImage()));
	rayLayout->addWidget(saveButton);

	layout->addWidget(rayGroupBox);
	layout->addStretch(0);
}

void Window::initControlWidget () {
    controlWidget = new QGroupBox ();
    QVBoxLayout * layout = new QVBoxLayout (controlWidget);
    
    QGroupBox * previewGroupBox = new QGroupBox ("Preview", controlWidget);
    QVBoxLayout * previewLayout = new QVBoxLayout (previewGroupBox);
    
    QCheckBox * wireframeCheckBox = new QCheckBox ("Wireframe", previewGroupBox);
    connect (wireframeCheckBox, SIGNAL (toggled (bool)), viewer, SLOT (setWireframe (bool)));
    previewLayout->addWidget (wireframeCheckBox);
   
    QButtonGroup * modeButtonGroup = new QButtonGroup (previewGroupBox);
    modeButtonGroup->setExclusive (true);
    QRadioButton * flatButton = new QRadioButton ("Flat", previewGroupBox);
    QRadioButton * smoothButton = new QRadioButton ("Smooth", previewGroupBox);
    modeButtonGroup->addButton (flatButton, static_cast<int>(GLViewer::Flat));
    modeButtonGroup->addButton (smoothButton, static_cast<int>(GLViewer::Smooth));
    connect (modeButtonGroup, SIGNAL (buttonClicked (int)), viewer, SLOT (setRenderingMode (int)));
    previewLayout->addWidget (flatButton);
    previewLayout->addWidget (smoothButton);
    
    QPushButton * snapshotButton  = new QPushButton ("Save preview", previewGroupBox);
    connect (snapshotButton, SIGNAL (clicked ()) , this, SLOT (exportGLImage ()));
    previewLayout->addWidget (snapshotButton);

    layout->addWidget (previewGroupBox);
    
    //Filter
    QGroupBox* filterGroupBox = new QGroupBox("Filters", controlWidget);
    QVBoxLayout* filterLayout = new QVBoxLayout(filterGroupBox);

    QLabel* gaussianFilterLabel = new QLabel("Gaussian Filter", filterGroupBox);
    QButtonGroup* filterButtonGroup = new QButtonGroup(filterGroupBox);
    QRadioButton* disabledGaussianButton = new QRadioButton("Disabled", filterGroupBox);
    enabledGaussianButton = new QRadioButton("Enabled", filterGroupBox);
    filterButtonGroup->addButton(disabledGaussianButton, static_cast<int>(RayTracer::GaussianFilterDisabled)); 
    filterButtonGroup->addButton(enabledGaussianButton, static_cast<int>(RayTracer::GaussianFilterEnabled)); 
    disabledGaussianButton->setChecked(true);
    connect(filterButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(setGaussianFilterMode(int)));

    QHBoxLayout* standardDeviationLayout = new QHBoxLayout();
    QLabel* standardDeviationLabel0 = new QLabel("Standard Deviation", filterGroupBox);
    QSlider* standardDeviationSlider = new QSlider(Qt::Horizontal, filterGroupBox);
    standardDeviationSlider->setRange(1, 50);
    standardDeviationSlider->setValue(1);
    standardDeviationGaussLabel = new QLabel("0.1", filterGroupBox);
    connect(standardDeviationSlider, SIGNAL(valueChanged(int)), this, SLOT(setStandardDeviationGauss(int)));
    standardDeviationLayout->addWidget(standardDeviationLabel0);
    standardDeviationLayout->addWidget(standardDeviationSlider);
    standardDeviationLayout->addWidget(standardDeviationGaussLabel);
    
    QHBoxLayout* sizeMaskLayout = new QHBoxLayout();
    QLabel* sizeMaskLabel0 = new QLabel("Size Mask", filterGroupBox);
    QSlider* sizeMaskSlider = new QSlider(Qt::Horizontal, filterGroupBox);
    sizeMaskSlider->setRange(1, 10);
    sizeMaskSlider->setValue(1);
    sizeMaskLabel = new QLabel("3", filterGroupBox);
    connect(sizeMaskSlider, SIGNAL(valueChanged(int)), this, SLOT(setSizeMask(int)));
    sizeMaskLayout->addWidget(sizeMaskLabel0);
    sizeMaskLayout->addWidget(sizeMaskSlider);
    sizeMaskLayout->addWidget(sizeMaskLabel);

    filterLayout->addWidget(gaussianFilterLabel);
    filterLayout->addWidget(disabledGaussianButton);
    filterLayout->addWidget(enabledGaussianButton);
    filterLayout->addLayout(standardDeviationLayout);
    filterLayout->addLayout(sizeMaskLayout);
    layout->addWidget(filterGroupBox);    
    
        
    QGroupBox * globalGroupBox = new QGroupBox ("Global Settings", controlWidget);
    QVBoxLayout * globalLayout = new QVBoxLayout (globalGroupBox);
    
    QPushButton * bgColorButton  = new QPushButton ("Background Color", globalGroupBox);
    connect (bgColorButton, SIGNAL (clicked()) , this, SLOT (setBGColor()));
    globalLayout->addWidget (bgColorButton);
    
    QPushButton * aboutButton  = new QPushButton ("About", globalGroupBox);
    connect (aboutButton, SIGNAL (clicked()) , this, SLOT (about()));
    globalLayout->addWidget (aboutButton);
    
    QPushButton * quitButton  = new QPushButton ("Quit", globalGroupBox);
    connect (quitButton, SIGNAL (clicked()) , qApp, SLOT (closeAllWindows()));
    globalLayout->addWidget (quitButton);

    layout->addWidget (globalGroupBox);

    layout->addStretch (0);
}
