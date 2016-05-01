/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "glwidget.h"
#include "material.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_xRot(0),
      m_yRot(0),
      m_zRot(0),
      m_program(0),
      obj(QCoreApplication::applicationDirPath().toStdString() + "/olento_testi.obj"),
      vertexBuffer(QOpenGLBuffer::VertexBuffer),
      normalBuffer(QOpenGLBuffer::VertexBuffer),
      elementBuffer(QOpenGLBuffer::IndexBuffer)
{
    cerr << "GLWidget constructor called succesfully\n";

/*
     m_core = QCoreApplication::arguments().contains(QStringLiteral("--coreprofile"));
    // --transparent causes the clear color to be transparent. Therefore, on systems that
    // support it, the widget will become transparent apart from the logo.
    m_transparent = QCoreApplication::arguments().contains(QStringLiteral("--transparent"));
    if (m_transparent) {
        QSurfaceFormat fmt = format();
        fmt.setAlphaBufferSize(8);
        setFormat(fmt);
    }
    */
}

GLWidget::~GLWidget()
{
    cleanup();
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void GLWidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_xRot) {
        m_xRot = angle;
        emit xRotationChanged(angle);
        update();
    }
}

void GLWidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_yRot) {
        m_yRot = angle;
        emit yRotationChanged(angle);
        update();
    }
}

void GLWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != m_zRot) {
        m_zRot = angle;
        emit zRotationChanged(angle);
        update();
    }
}

void GLWidget::cleanup()
{
    makeCurrent();
    vertexBuffer.destroy();
    normalBuffer.destroy();
    elementBuffer.destroy();
    delete m_program;
    m_program = 0;
    doneCurrent();
}

void GLWidget::setData(dObject &obj) {
    vertexBuffer.bind();
    vertexBuffer.allocate(obj.getVertexData().data,obj.getVertexData().length);

    normalBuffer.bind();
    normalBuffer.allocate(obj.getNormalData().data,obj.getNormalData().length);

    elementBuffer.bind();
    elementBuffer.allocate(obj.getElementData().data,obj.getElementData().length);
    elements_n = obj.elements.size();
}

void GLWidget::initializeGL()
{
    // In this example the widget's corresponding top-level window can change
    // several times during the widget's lifetime. Whenever this happens, the
    // QOpenGLWidget's associated context is destroyed and a new one is created.
    // Therefore we have to be prepared to clean up the resources on the
    // aboutToBeDestroyed() signal, instead of the destructor. The emission of
    // the signal will be followed by an invocation of initializeGL() where we
    // can recreate all resources.

    std::cerr << "Initialize GL ...";

    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);

    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

    m_program = new QOpenGLShaderProgram;

    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, QCoreApplication::applicationDirPath() + "/StandardShading.vertexshader");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, QCoreApplication::applicationDirPath() + "/StandardShading.fragmentshader");

    m_program->bindAttributeLocation("vertexPosition_modelspace", 0);
    m_program->bindAttributeLocation("vertexNormal_modelspace", 1);

    m_program->link();
    m_program->bind();


    mvpId = m_program->uniformLocation("MVP");
    viewId = m_program->uniformLocation("V");
    modelId = m_program->uniformLocation("M");
    lightId = m_program->uniformLocation("LightPosition_worldspace");

    DiffuseId = m_program->uniformLocation("diffuseColor");
    SpecularId = m_program->uniformLocation("specularity");
    HardnessId = m_program->uniformLocation("hardness");
    AlphaId = m_program->uniformLocation("alpha");

    // aspect ratio
    aspectRatio = 4/3;

    //lasketaan projekti matriisi
    proj.perspective(45.0f, aspectRatio, 0.1f, 100.0f);


    // set view matrix
    QVector3D position(8,3,3);
    QVector3D target(0,0,0);
    QVector3D up(0,1,0);

    // feed matrix.
    view.lookAt(
                position,
                target,
                up
                );

    model = QMatrix4x4();

    // Create a vertex array object. In OpenGL ES 2.0 and OpenGL 2.x
    // implementations this is optional and support may not be present
    // at all. Nonetheless the below code works in all cases and makes
    // sure there is a VAO when one is needed.
    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    // Setup our vertex buffer object.
    vertexBuffer.create();
    normalBuffer.create();
    elementBuffer.create();

    for(int i = 0; i < 100; i++)
        std::cout << obj.elements[i] << ", ";
    std::cout << "\n";

    setData(obj);

    // Store the vertex attribute bindings for the program.
    //setupVertexAttribs();


    // Light position is fixed.
    m_program->setUniformValue(lightId, QVector3D(11, 6, 11));

    m_program->release();

    std::cerr << "ok\n";
}


void GLWidget::setupVertexAttribs()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    vertexBuffer.bind();

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    vertexBuffer.release();

    normalBuffer.bind();
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    normalBuffer.release();
}

void GLWidget::paintGL()
{
    std::cerr << "PaintGL ...";

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    /*
    m_world.setToIdentity();
    m_world.rotate(180.0f - (m_xRot / 16.0f), 1, 0, 0);
    m_world.rotate(m_yRot / 16.0f, 0, 1, 0);
    m_world.rotate(m_zRot / 16.0f, 0, 0, 1);
*/

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_program->bind();
    m_program->setUniformValue(modelId, model);
    m_program->setUniformValue(viewId, view);
    m_program->setUniformValue(projId, proj);
    m_program->setUniformValue(mvpId, proj * view * QMatrix4x4()); // matrix default constructor represents model matrix

    //testataan materiaali 0:lla
    material M = getMaterial(0);

    //m_program->setUniformValue(DiffuseId, M.diffuseColor);
    m_program->setUniformValue(DiffuseId, QVector3D(M.diffuseColor.r, M.diffuseColor.g, M.diffuseColor.b));
    m_program->setUniformValue(SpecularId, M.specularity);
    m_program->setUniformValue(HardnessId, M.hardness);
    m_program->setUniformValue(AlphaId, M.alpha);

    setupVertexAttribs();

    glDrawElements(GL_TRIANGLES,elements_n,GL_UNSIGNED_INT,0);

    m_program->release();

    std::cerr << "ok\n";
}

void GLWidget::resizeGL(int w, int h)
{
    proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(m_xRot + 8 * dy);
        setYRotation(m_yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(m_xRot + 8 * dy);
        setZRotation(m_zRot + 8 * dx);
    }
    m_lastPos = event->pos();
}
