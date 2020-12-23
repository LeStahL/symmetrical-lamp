#pragma once

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>

class OpenGLWidget : public QOpenGLWidget
{
    static inline QString UNIFORM_BLOCK = R"###(
uniform vec2 iResolution;
uniform float iTime;
uniform float topDial0;
uniform float topDial1;
uniform float topDial2;
uniform float topDial3;
uniform float topDial4;
uniform float topDial5;
uniform float topDial6;
uniform float topDial7;
uniform float rightDial0;
uniform float rightDial1;
uniform float rightDial2;
uniform float rightDial3;
uniform float rightDial4;
uniform float rightDial5;
uniform float rightDial6;
uniform float rightDial7;
uniform float iFader0;
uniform float iFader1;
uniform float iFader2;
uniform float iFader3;
uniform float iFader4;
uniform float iFader5;
uniform float iFader6;
uniform float iFader7;
)###";

    static inline QString DEFAULT_SHADER_SOURCE = R"###(
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;

    // Time varying pixel color
    vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));

    // Output to screen
    fragColor = vec4(col,1.0);
}
)###";

    static inline QString ENTRY_POINT = R"###(
void main()
{
    mainImage(gl_FragColor, gl_FragCoord.xy);
}
)###";

    static inline QString VERTEX_SHADER = R"###(
#version 460 core

out vec3 gl_Position;
layout(location = 0) in vec3 position;

void main()
{
    gl_Position = position;
}
)###";

    static inline GLfloat VERTICES[] = {
        -1.f, -1.f, 0.0f,
        -1.f, 1.f, 0.0f,
        1.f, -1.f, 0.0f,
        -1.f, 1.f, 0.0f,
        1.f, -1.f, 0.0f,
        1.f, 1.f, 0.f
    };

    QOpenGLShaderProgram *program;
    QOpenGLBuffer vertexBufferObject;
    QOpenGLVertexArrayObject vertexArrayObject;

public:
    OpenGLWidget(QWidget *parent = nullptr);

    int width,
        height;

    bool monitorFileWithName(QString fileName = "");

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
};