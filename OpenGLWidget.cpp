#include "OpenGLWidget.h"

#include <QOpenGLFunctions>
#include <QDebug>

#include "MidiController.h"

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    QList<MidiController *> availableControllers = MidiController::availableControllers();
    for(int i=0; i<availableControllers.size(); ++i)
        qDebug() << availableControllers.at(i)->inputKey
            << availableControllers.at(i)->outputKey 
            << availableControllers.at(i)->name
            << (availableControllers.at(i)->type & MidiController::InputController ? "Input":"")
            << (availableControllers.at(i)->type & MidiController::OutputController ? "Output":"");

    QSurfaceFormat format;
    format.setVersion(4,6);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
}

void OpenGLWidget::initializeGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    this->program = new QOpenGLShaderProgram(QOpenGLContext::currentContext());
    this->program->addShaderFromSourceCode(QOpenGLShader::Fragment, OpenGLWidget::UNIFORM_BLOCK + OpenGLWidget::DEFAULT_SHADER_SOURCE + OpenGLWidget::ENTRY_POINT);
    this->program->addShaderFromSourceCode(QOpenGLShader::Vertex, OpenGLWidget::VERTEX_SHADER);
    if(!this->program->link())
    {
        qDebug() << this->program->log();
        return;
    }

    this->program->bind();

    this->vertexBufferObject.create();
    this->vertexBufferObject.bind();
    this->vertexBufferObject.setUsagePattern(QOpenGLBuffer::StaticDraw);
    this->vertexBufferObject.allocate(OpenGLWidget::VERTICES, sizeof(OpenGLWidget::VERTICES));

    this->vertexArrayObject.create();
    this->vertexArrayObject.bind();

    this->program->enableAttributeArray(0);
    this->program->setAttributeBuffer(0, GL_FLOAT, 0, 3);

    // vertexBufferObject.release();
    // vertexArrayObject.release();
}

void OpenGLWidget::paintGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClear(GL_COLOR_BUFFER_BIT);

    // vertexBufferObject.bind();
    // vertexArrayObject.bind();
    // loadingBarProgram->setUniformValue("iProgress", (GLfloat).75);
    // loadingBarProgram->setUniformValue("iResolution", size());

    f->glDrawArrays(GL_TRIANGLES, 0, 6);

    // loadingBarProgram->disableAttributeArray("position");
    // vertexBufferObject.release();
    // vertexArrayObject.release();
}

void OpenGLWidget::resizeGL(int width, int height) 
{
    this->width = width;
    this->height = height;
    this->program->setUniformValue("iResolution", QPointF(width, height));
}

// // TODO (ak): fix this crap
bool OpenGLWidget::monitorFileWithName(QString fileName) {
//     return addShaderFromSourceFile(QOpenGLShader::Fragment, fileName);
    return true;
}