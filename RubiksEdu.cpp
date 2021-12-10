#include "RubiksEdu.h"
#include "ui_RubiksEdu.h"
#include <iostream>
#include <QMenuBar>
#include <QMainWindow>
#include <QAction>

RubiksEdu::RubiksEdu(QWidget *parent, CubeController *controller, TutorialBrowser *tutorial)
    : QMainWindow(parent)
    , ui(new Ui::RubiksEdu)
{
    ui->setupUi(this);
    connect(this,&RubiksEdu::sendMove,controller,&CubeController::MoveCube);
    connect(controller,&CubeController::updateCube,this, &RubiksEdu::displayCube);
    connect(ui->cubeWidget,SIGNAL(faceSelected(int)),controller,SLOT(switchFace(int)));
        // TODO: delete, this is for testing:
    // Tutorial Widget Connections
    connect(ui->tutorialTextBrowser, &TutorialBrowser::tutorialStepChanged, this, [] (int step) {
        qDebug() << step;
    });
    connect(ui->stepSelect, &QComboBox::currentIndexChanged, this, [=] (int stepIndex) {
        ui->tutorialTextBrowser->setTutorialStep(stepIndex + 1);
    });
    connect(ui->tutorialTextBrowser, &TutorialBrowser::tutorialStepChanged, this, [=] (int stepID) {
        ui->stepSelect->setCurrentIndex(stepID-1);
    });
    connect(ui->homeButton, &QPushButton::pressed, this, [=] () {
       ui->tutorialTextBrowser->setTutorialStep(-1);
       ui->stepSelect->setCurrentIndex(-1);
    });
    connect(tutorial, &TutorialBrowser::tutorialStepChanged, controller, [=] () {
       &CubeController::setEduMode;
    });

    // connect check button
    connect(ui->checkButton, &QPushButton::clicked, controller, &CubeController::checkCompletion);
    // connection for showing the celebration
    connect(controller, &CubeController::complete, this, &RubiksEdu::showCelebration);

    //3d view stuff --------------------------------------------
    scene3D= new QGraphicsScene;

    createLeftOfCube();
    createRightOfCube();
    createTopOfCube();

    scene3D->setBackgroundBrush(Qt::white);
    ui->cube3DGraphicsView->setScene(scene3D);
    ui->cube3DGraphicsView->scale(0.75,-0.75);

    connectControllerView(controller);
    //end of 3d view stuff ----------------------------------


    // Displays initial cube  (3D view stuff needs to happen before this so the 3D view gets painted with 2D data)
    on_leftTopButton_clicked();
    on_rightTopButton_clicked();
}
RubiksEdu::~RubiksEdu()
{
    delete ui;
}


void RubiksEdu::on_topLeftButton_clicked()
{
    emit sendMove(7);
}


void RubiksEdu::on_topRightButton_clicked()
{
    emit sendMove(6);
}


void RubiksEdu::on_leftTopButton_clicked()
{
    emit sendMove(0);
}


void RubiksEdu::on_leftBttmButton_clicked()
{
    emit sendMove(1);
}


void RubiksEdu::on_bttmLeftButton_clicked()
{
    emit sendMove(2);
}


void RubiksEdu::on_bttmRightButton_clicked()
{
    emit sendMove(3);
}


void RubiksEdu::on_rightBttmButton_clicked()
{
    emit sendMove(4);
}


void RubiksEdu::on_rightTopButton_clicked()
{
    emit sendMove(5);
}

void RubiksEdu::displayCube(std::vector<QImage> faces)
{
    QImage farRightFace = faces.back();
    faces.pop_back();
    QImage bottomFace = faces.back();
    faces.pop_back();
    QImage rightFace = faces.back();
    faces.pop_back();
    QImage topFace = faces.back();
    faces.pop_back();
    QImage leftFace = faces.back();
    faces.pop_back();
    QImage centerFace = faces.back();
    faces.pop_back();
    QImage centerFaceScaled = centerFace.scaled(120,120);
    QImage rightFaceScaled = rightFace.scaled(120,120);
    QImage leftFaceScaled = leftFace.scaled(120,120);
    QImage topFaceScaled = topFace.scaled(120,120);
    QImage bottomFaceScaled = bottomFace.scaled(120,120);
    QImage farRightFaceScaled = farRightFace.scaled(120,120);

    ui->face0->setPixmap(QPixmap::fromImage(centerFaceScaled));
    ui->face1->setPixmap(QPixmap::fromImage(leftFaceScaled));
    ui->face2->setPixmap(QPixmap::fromImage(topFaceScaled));
    ui->face3->setPixmap(QPixmap::fromImage(rightFaceScaled));
    ui->face4->setPixmap(QPixmap::fromImage(bottomFaceScaled));
    ui->face5->setPixmap(QPixmap::fromImage(farRightFaceScaled));
    ui->face0->setScaledContents( true );
    ui->face0->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );

}

void RubiksEdu::on_clockwiseButton_clicked()
{
    emit sendMove(8);
}


void RubiksEdu::on_counterClockwiseButton_clicked()
{
    emit sendMove(9);
}


// 3D Cube View code starts here--------------------------------------------------------------------------
/*
 *
 */
void RubiksEdu::cube3DpaintVisibleFacesSimple(std::vector<char> &visibleFaces){
    //qDebug()<< "cube3DpainVisibleFacesSimple" << visibleFaces.size();
    repaintLeftFace(getColorFromChar(visibleFaces.at(0)));
    repaintRightFace(getColorFromChar(visibleFaces.at(1)));
    repaintTopFace(getColorFromChar(visibleFaces.at(2)));
}

/*
 *
 */
void RubiksEdu::cube3DpaintVisibleFaces(QVector<CubeFace> &visibleFaces){
    //qDebug()<< "cube3DpainVisibleFaces" << visibleFaces.size();
    repaintLeftFace(visibleFaces.at(0));
    repaintRightFace(visibleFaces.at(1));
    repaintTopFace(visibleFaces.at(2));
}

/*
 *
 */
QColor RubiksEdu::getColorFromChar(char charColor){

    switch(charColor){
        case 'r':
            return Qt::red;
        case 'g':
            return QColor("green");//Qt::green;
        case 'b':
            return Qt::blue;
        case 'w':
            return Qt::white;
        case 'o':
            return QColor("orange");//QColor(255,165,0);
        case 'y':
            return Qt::yellow;
        default:
            return Qt::gray;
    }
}

/*
 *3d
*/
void RubiksEdu::repaintLeftFace(CubeFace faceToPaint){
    for(int xPos = 0; xPos<3; xPos++)
    {
        int xPosCorrected = 2-xPos; //change xPosition so that 0 element is on left and 2 element is on right.
                                    //This ensures the face 0,0 point is on the bottom left corner for every face
        std::vector<char> column = faceToPaint.getCol(xPosCorrected);

        for(int yPos = 0; yPos<3; yPos++)
        {
            QColor currentColor = getColorFromChar(column.at(yPos));

            int vctrIndex = xPosCorrected*3+yPos;
            QGraphicsPathItem *tempItemReference = vctrVisibleFaceLeft.at(vctrIndex);
            tempItemReference->setBrush(currentColor);
        }
    }
}

/*
 *3d
*/
void RubiksEdu::repaintRightFace(CubeFace faceToPaint){
    for(int xPos = 0; xPos<3; xPos++)
    {
        std::vector<char> column = faceToPaint.getCol(xPos);

        for(int yPos = 0; yPos<3; yPos++)
        {
            QColor currentColor = getColorFromChar(column.at(yPos));

            int vctrIndex = xPos*3+yPos;
            QGraphicsPathItem *tempItemReference = vctrVisibleFaceRight.at(vctrIndex);
            tempItemReference->setBrush(currentColor);
        }
    }
}

/*
 *3d
*/
void RubiksEdu::repaintTopFace(CubeFace faceToPaint){
    for(int xPos = 0; xPos<3; xPos++)
    {
        std::vector<char> column = faceToPaint.getCol(xPos);

        for(int yPos = 0; yPos<3; yPos++)
        {
            QColor currentColor = getColorFromChar(column.at(yPos));

            int vctrIndex = xPos*3+yPos;
            QGraphicsPathItem *tempItemReference = vctrVisibleFaceTop.at(vctrIndex);
            tempItemReference->setBrush(currentColor);
        }
    }
}

/*
 *3d
*/
void RubiksEdu::repaintLeftFace(QColor colorToPaint){
    for(int xPos = 0; xPos<3; xPos++)
    {
        int xPosCorrected = 2-xPos; //change xPosition so that 0 element is on left and 2 element is on right.
                                    //This ensures the face 0,0 point is on the bottom left corner for every face
        for(int yPos = 0; yPos<3; yPos++)
        {
            int vctrIndex = xPosCorrected*3+yPos;
            QGraphicsPathItem *tempItemReference = vctrVisibleFaceLeft.at(vctrIndex);
            tempItemReference->setBrush(colorToPaint);
        }
    }
}

/*
 *3d
*/
void RubiksEdu::repaintRightFace(QColor colorToPaint){
    for(int xPos = 0; xPos<3; xPos++)
    {
        for(int yPos = 0; yPos<3; yPos++)
        {
            int vctrIndex = xPos*3+yPos;
            QGraphicsPathItem *tempItemReference = vctrVisibleFaceRight.at(vctrIndex);
            tempItemReference->setBrush(colorToPaint);
        }
    }
}

/*
 *3d
*/
void RubiksEdu::repaintTopFace(QColor colorToPaint){
    for(int xPos = 0; xPos<3; xPos++)
    {
        for(int yPos = 0; yPos<3; yPos++)
        {
            int vctrIndex = xPos*3+yPos;
            QGraphicsPathItem *tempItemReference = vctrVisibleFaceTop.at(vctrIndex);
            tempItemReference->setBrush(colorToPaint);
        }
    }
}

/*
 *3d
*/
void RubiksEdu::on_pushLeftBtn_clicked()
{
    emit send3DRotation("Left");
}

/*
 *3d
*/
void RubiksEdu::on_pushRightBtn_clicked()
{
        emit send3DRotation("Right");
}

/*
 *3d
*/
void RubiksEdu::on_pushUpBtn_clicked()
{
    emit send3DRotation("Up");
}

/*
 *3d
*/
void RubiksEdu::connectControllerView(CubeController *controller){
    connect(this, &RubiksEdu::send3DRotation,controller,&CubeController::rotationCube);
    connect(controller, &CubeController::update3DCubeViewSimple, this, &RubiksEdu::cube3DpaintVisibleFacesSimple);
    connect(controller, &CubeController::update3DCubeView, this, &RubiksEdu::cube3DpaintVisibleFaces);
}

/*
 *left side
*/
void RubiksEdu::createLeftOfCube(){

    for(int xPos = 0; xPos<3; xPos++)
    {
        int xPosCorrected = 2-xPos; //change xPosition so that 0 element is on left and 2 element is on right.
                                    //This ensures the face 0,0 point is on the bottom left corner for every face
        for(int yPos = 0; yPos<3; yPos++)
        {
            point1x = -eSize*xPosCorrected*xScale;
            point1y = eSize*yPos+eSize*xPosCorrected*yScale;
            point2x = -eSize*(xPosCorrected+1)*xScale;
            point2y = eSize*yPos+eSize*yScale+eSize*xPosCorrected*yScale;
            point3x = -eSize*(xPosCorrected+1)*xScale;
            point3y = eSize*(yPos+1)+eSize*yScale+eSize*xPosCorrected*yScale;
            point4x = -eSize*xPosCorrected*xScale;
            point4y = eSize*(yPos+1)+eSize*xPosCorrected*yScale;

            QPainterPath path1;

            path1.moveTo(point1x, point1y);
            path1.lineTo(point2x, point2y);
            path1.lineTo(point3x, point3y);
            path1.lineTo(point4x, point4y);
            path1.lineTo(point1x, point1y);
            QGraphicsPathItem *tempItemReference;
            tempItemReference = scene3D->addPath(path1);
            vctrVisibleFaceLeft.append(tempItemReference);

            tempItemReference->setBrush(Qt::yellow);
        }
    }


}

/*
 *right side
*/
void RubiksEdu::createRightOfCube(){
    for(int xPos = 0; xPos<3; xPos++)
    {
        for(int yPos = 0; yPos<3; yPos++)
        {
            point1x = eSize*xPos*xScale;
            point1y = eSize*yPos+eSize*xPos*yScale;
            point2x = eSize*(xPos+1)*xScale;
            point2y = eSize*yPos+eSize*yScale+eSize*xPos*yScale;
            point3x = eSize*(xPos+1)*xScale;
            point3y = eSize*(yPos+1)+eSize*yScale+eSize*xPos*yScale;
            point4x = eSize*xPos*xScale;
            point4y = eSize*(yPos+1)+eSize*xPos*yScale;

            QPainterPath path2;

            path2.moveTo(point1x, point1y);
            path2.lineTo(point2x, point2y);
            path2.lineTo(point3x, point3y);
            path2.lineTo(point4x, point4y);
            path2.lineTo(point1x, point1y);

            QGraphicsPathItem *tempItemReference;
            tempItemReference = scene3D->addPath(path2);
            vctrVisibleFaceRight.append(tempItemReference);

            tempItemReference->setBrush(Qt::red);
        }

    }
}

/*
 *top side
*/
void RubiksEdu::createTopOfCube(){

    for(int xPos = 0; xPos<3; xPos++)
    {
        for(int yPos = 0; yPos<3; yPos++)
        {
            point1x = (xPos-yPos)*eSize*xScale;
            point1y = cubeSize+(xPos+yPos)*eSize*yScale;
            point2x = (xPos+1-yPos)*eSize*xScale;
            point2y = cubeSize+(xPos+yPos)*eSize*yScale+eSize*yScale;
            point3x = (xPos-yPos)*eSize*xScale;
            point3y = cubeSize+(xPos+yPos+1)*eSize*yScale+eSize*yScale;
            point4x = (xPos-1-yPos)*eSize*xScale;
            point4y = cubeSize+(xPos+yPos)*eSize*yScale+eSize*yScale;

            QPainterPath path3;

            path3.moveTo(point1x, point1y);
            path3.lineTo(point2x, point2y);
            path3.lineTo(point3x, point3y);
            path3.lineTo(point4x, point4y);
            path3.lineTo(point1x, point1y);


            QGraphicsPathItem *tempItemReference;
            tempItemReference = scene3D->addPath(path3);
            vctrVisibleFaceTop.append(tempItemReference);

            tempItemReference->setBrush(Qt::green);
        }
    }

}


void RubiksEdu::on_checkButton_clicked()
{
    emit checkButton();
}

void RubiksEdu::showCelebration(bool complete) {
    CubeCelebration c;
    c.exec();
}

