#include <stdio.h>                  // ��� ������������ �����/������ (printf � �.�.)
#include <GL/freeglut.h>           // ������������ ���� freeglut 
#include "chip_8.h"                 // ������������ ���� ������ ��������� Chip-8
#include <chrono> // ��� ������� ��������� �������

// ������� ������ Chip-8
#define SCREEN_WIDTH 64              // ������ ������: 64 �������
#define SCREEN_HEIGHT 32             // ������ ������: 32 �������

chip8 myChip8;                      // ������� ��������� ��������� Chip-8
int modifier = 10;                   // ���������� ����������� ��� ���������� ������� ����

// ������� ���� (����������� �� ������ �������� ������ � ����������� ������������)
int display_width = SCREEN_WIDTH * modifier;    // ������ ����
int display_height = SCREEN_HEIGHT * modifier;   // ������ ����

// ����� �� ��������� (� ������� RGB: �������, �������, �����)
float backgroundColor[3] = { 0.0f, 0.0f, 0.0f }; // ������ ���� ����
float pixelColor[3] = { 1.0f, 1.0f, 1.0f };       // ����� ���� ��������

// ���������� ������� (����������� ����� ����)
void display();                     // ������� ��������� (���������� ��� ���������� ������)
void reshape_window(GLsizei w, GLsizei h); // ������� ��������� ������� ����
void keyboardUp(unsigned char key, int x, int y);   // ������� ��������� ���������� �������
void keyboardDown(unsigned char key, int x, int y); // ������� ��������� ������� �������

// ������������ ����� ����� ��������� (�� ������ �������)
#define DRAWWITHTEXTURE 

typedef unsigned __int8 u8;          // ���������� ��� u8 ��� unsigned char (8-������ ����� ��� �����)
u8 screenData[SCREEN_HEIGHT][SCREEN_WIDTH][3]; // ������ ��� �������� ������ �������� (RGB)
void setupTexture();                // ������� ��������� ��������

int main(int argc, char** argv) {
    if (argc < 2) {                 // ���� �� ������� ��� ����� ROM � ���������� ��������� ������
        printf("Usage: myChip8.exe chip8application\n\n"); // ������� ��������� �� �������������
        return 1;                   // ��������� ��������� � ����� ������ 1
    }

    // �������� ROM-����� (����)
    if (!myChip8.loadApplication(argv[1])) { // �������� ��������� ����������
        printf("Failed to load application.\n"); // ���� �������� �� �������
        return 1;                   // ��������� ��������� � ����� ������ 1
    }

    // ��������� OpenGL (������������� GLUT)
    glutInit(&argc, argv);             // ������������� GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ������� ����������� � RGBA-������ �����

    glutInitWindowSize(display_width, display_height); // ��������� �������� ����
    glutInitWindowPosition(320, 320);               // ��������� ������� ����
    glutCreateWindow("myChip8");   // �������� ���� � ����������

    // ����������� ������� ��������� ������ (callbacks)
    glutDisplayFunc(display);           // ������� ���������
    glutIdleFunc(display);              // �������, ���������� � ������ �������� (��� ��������)
    glutReshapeFunc(reshape_window);    // ������� ��������� ������� ����
    glutKeyboardFunc(keyboardDown);     // ������� ��������� ������� �������
    glutKeyboardUpFunc(keyboardUp);       // ������� ��������� ���������� �������

#ifdef DRAWWITHTEXTURE
    setupTexture();                 // ��������� ��������, ���� �������� ��������� � ����������
#endif

    glutMainLoop();                     // ������ ��������� ����� GLUT (��������� �������)

    return 0;                           // ���������� ��������� � ����� ������ 0
}

// ��������� �������� (������������� ������ ��������)
void setupTexture() {
    // ������� ������ ������ (���������� ������ = ������ ����)
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
        for (int x = 0; x < SCREEN_WIDTH; ++x)
            screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;

    // �������� �������� OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

    // ��������� ���������� �������� (����������, �����������)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Nearest-neighbor ����������
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Nearest-neighbor ����������
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);     // ������ ����������� �� �����������
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);     // ������ ����������� �� ���������

    // ��������� ���������������
    glEnable(GL_TEXTURE_2D);
}

// ���������� �������� (������� ������ �� ��������� �� �����)
void updateTexture(const chip8& c8) {
    // ���������� �������� � ������ �������� �� ������ ��������� ���������
    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 64; ++x)
            if (c8.gfx[(y * 64) + x] == 0) { // ���� ������� ��������
                screenData[y][x][0] = static_cast<unsigned char>(backgroundColor[0] * 255);// ������������� ���� ����
                screenData[y][x][1] = static_cast<unsigned char>(backgroundColor[1] * 255);
                screenData[y][x][2] = static_cast<unsigned char>(backgroundColor[2] * 255);
            }
            else {                            // ���� ������� �������
                screenData[y][x][0] = static_cast<unsigned char>(pixelColor[0] * 255);// ������������� ���� �������
                screenData[y][x][1] = static_cast<unsigned char>(pixelColor[1] * 255);
                screenData[y][x][2] = static_cast<unsigned char>(pixelColor[2] * 255);
            }

    // �������� ����������� ������ � �������� OpenGL
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

    // ��������� ����� ������� ������ (����)
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 0.0f);

    // ��������� ��������, �� ������� ������������� ��������
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0); glVertex2d(0.0, 0.0); // ����� ������� ����
    glTexCoord2d(1.0, 0.0); glVertex2d(display_width, 0.0); // ������ ������� ����
    glTexCoord2d(1.0, 1.0); glVertex2d(display_width, display_height); // ������ ������ ����
    glTexCoord2d(0.0, 1.0); glVertex2d(0.0, display_height); // ����� ������ ����
    glEnd();
}

// ������ ��� ������� (��������� ����������)
void drawPixel(int x, int y) {
    glBegin(GL_QUADS);
    glColor3f(pixelColor[0], pixelColor[1], pixelColor[2]); // ��������� ����� �������
    glVertex3f((static_cast<GLfloat>(x) * modifier) + 0.0f, (static_cast<GLfloat>(y) * modifier) + 0.0f, 0.0f);    // ����� ������� ����: ���������� ���������� ������ �������� ���� ��������. x � y ���������� �� modifier ��� ���������������.
    glVertex3f((static_cast<GLfloat>(x) * modifier) + 0.0f, (static_cast<GLfloat>(y) * modifier) + modifier, 0.0f); // ����� ������ ����: ���������� ���������� ������ ������� ���� ��������. y ������������� �� modifier, ����� ���������� ����.
    glVertex3f((static_cast<GLfloat>(x) * modifier) + modifier, (static_cast<GLfloat>(y) * modifier) + modifier, 0.0f); // ������ ������ ����: ���������� ���������� ������� ������� ���� ��������. x � y ������������� �� modifier.
    glVertex3f((static_cast<GLfloat>(x) * modifier) + modifier, (static_cast<GLfloat>(y) * modifier) + 0.0f, 0.0f);    // ������ ������� ����: ���������� ���������� ������� �������� ���� ��������. x ������������� �� modifier.
    glEnd();
}

// ���������� ������, ����� �������� ��� ������� �������
void updateQuads(const chip8& c8) {
    // ������� ���� �������� ������
    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 64; ++x) {
            if (c8.gfx[(y * 64) + x] == 0)  // ���� ������� ��������
                glColor3f(backgroundColor[0], backgroundColor[1], backgroundColor[2]); // ������������� ���� ����
            else                               // ���� ������� �������
                glColor3f(pixelColor[0], pixelColor[1], pixelColor[2]); // ������������� ���� �������

            drawPixel(x, y); // ������ ������� ��� �������� �������
        }
}

const int cyclesPerFrame = 2; // ���������� ������ Chip-8 �� ���� ���� ���������

// ������� ��������� (���������� ��� ���������� ������)
void display() {
    static auto lastTime = std::chrono::high_resolution_clock::now(); // ��������� ����� ���������� ����� (static ��� ���������� ����� ��������)
    auto currentTime = std::chrono::high_resolution_clock::now(); // �������� ������� �����
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime); // ��������� �����, ��������� � ���������� �����

    const int targetFrameTime = 1000 / 60; // ������� ����� ��� ������ ����� (60 FPS)

    if (duration.count() >= targetFrameTime) { // ���� ������ ���������� ������� ��� ��������� ������ �����
        lastTime = currentTime; // ��������� ����� ���������� �����

        // ��������� ��������� ������ �������� Chip-8 �� ����
        for (int i = 0; i < cyclesPerFrame; ++i) {
            myChip8.emulateCycle(); // ��������� ���� ���� Chip-8
        }

        if (myChip8.drawFlag) { // ���� Chip-8 �������� ���������� ������
            glClear(GL_COLOR_BUFFER_BIT); // ������� ����� �����

#ifdef DRAWWITHTEXTURE
            updateTexture(myChip8); // ��������� �������� ������
#else
            updateQuads(myChip8); // ��������� �����, ����� ��������
#endif

            glutSwapBuffers(); // ������ ������ (��� ������� �����������)
            myChip8.drawFlag = false; // ���������� ���� ���������
        }
    }
}

// ������� ��������� ������� ����
void reshape_window(GLsizei w, GLsizei h) {
    // ��������� ����� ����
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 0.0f);

    // ��������� ������� �������� OpenGL
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0); // ��������������� �������� (2D)
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);

    // ���������� �������� ����
    display_width = w;
    display_height = h;
}

// ������� ��������� ������� �������
void keyboardDown(unsigned char key, int x, int y) {
    printf("keyboardDown: key=%c\n", key);// ���������� �����
    // ����������� ������ (��������� �������� �������)
    if (key == 'i')
        pixelColor[0] += 0.1f; // ����������� ������� ���������� ����� ��������
    else if (key == 'o')
        pixelColor[1] += 0.1f; // ����������� ������� ���������� ����� ��������
    else if (key == 'p')
        pixelColor[2] += 0.1f; // ����������� ����� ���������� ����� ��������
    else if (key == 'I')
        backgroundColor[0] += 0.1f; // ����������� ������� ���������� ����� ����
    else if (key == 'O')
        backgroundColor[1] += 0.1f; // ����������� ������� ���������� ����� ����
    else if (key == 'P')
        backgroundColor[2] += 0.1f; // ����������� ����� ���������� ����� ����

    // ����������� �������� ����� � ��������� �� 0.0 �� 1.0
    for (int i = 0; i < 3; ++i) {
        if (pixelColor[i] > 1.0f)
            pixelColor[i] = 0.0f; // ���� ������ 1.0, ��������
        else if (pixelColor[i] < 0.0f)
            pixelColor[i] = 1.0f; // ���� ������ 0.0, ������������� � 1.0
        if (backgroundColor[i] > 1.0f)
            backgroundColor[i] = 0.0f; // ���� ������ 1.0, ��������
        else if (backgroundColor[i] < 0.0f)
            backgroundColor[i] = 1.0f; // ���� ������ 0.0, ������������� � 1.0
    }

    // ������� Chip-8 (�������� ������ � ������� Chip-8)
    if (key == 27) // esc (�����)
        exit(0);   // ��������� ���������

    if (key == '1')
        myChip8.key[0x1] = 1;
    else if (key == '2')
        myChip8.key[0x2] = 1;
    else if (key == '3')
        myChip8.key[0x3] = 1;
    else if (key == '4')
        myChip8.key[0xC] = 1;

    else if (key == 'q')
        myChip8.key[0x4] = 1;
    else if (key == 'w')
        myChip8.key[0x5] = 1;
    else if (key == 'e')
        myChip8.key[0x6] = 1;
    else if (key == 'r')
        myChip8.key[0xD] = 1;

    else if (key == 'a')
        myChip8.key[0x7] = 1;
    else if (key == 's')
        myChip8.key[0x8] = 1;
    else if (key == 'd')
        myChip8.key[0x9] = 1;
    else if (key == 'f')
        myChip8.key[0xE] = 1;

    else if (key == 'z')
        myChip8.key[0xA] = 1;
    else if (key == 'x')
        myChip8.key[0x0] = 1;
    else if (key == 'c')
        myChip8.key[0xB] = 1;
    else if (key == 'v')
        myChip8.key[0xF] = 1;

    //printf("Press key %c\n", key);
}

// ������� ��������� ���������� �������
void keyboardUp(unsigned char key, int x, int y)
{
    // ���������� ������ Chip-8 (����� ��������� ������)
    if (key == '1')
        myChip8.key[0x1] = 0;
    else if (key == '2')
        myChip8.key[0x2] = 0;
    else if (key == '3')
        myChip8.key[0x3] = 0;
    else if (key == '4')
        myChip8.key[0xC] = 0;

    else if (key == 'q')
        myChip8.key[0x4] = 0;
    else if (key == 'w')
        myChip8.key[0x5] = 0;
    else if (key == 'e')
        myChip8.key[0x6] = 0;
    else if (key == 'r')
        myChip8.key[0xD] = 0;

    else if (key == 'a')
        myChip8.key[0x7] = 0;
    else if (key == 's')
        myChip8.key[0x8] = 0;
    else if (key == 'd')
        myChip8.key[0x9] = 0;
    else if (key == 'f')
        myChip8.key[0xE] = 0;

    else if (key == 'z')
        myChip8.key[0xA] = 0;
    else if (key == 'x')
        myChip8.key[0x0] = 0;
    else if (key == 'c')
        myChip8.key[0xB] = 0;
    else if (key == 'v')
        myChip8.key[0xF] = 0;
}