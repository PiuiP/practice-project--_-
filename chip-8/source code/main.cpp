#include <stdio.h>                  // Для стандартного ввода/вывода (printf и т.д.)
#include <GL/freeglut.h>           // Заголовочный файл freeglut 
#include "chip_8.h"                 // Заголовочный файл нашего эмулятора Chip-8
#include <chrono> // Для точного измерения времени

// Размеры экрана Chip-8
#define SCREEN_WIDTH 64              // Ширина экрана: 64 пикселя
#define SCREEN_HEIGHT 32             // Высота экрана: 32 пикселя

chip8 myChip8;                      // Создаем экземпляр эмулятора Chip-8
int modifier = 10;                   // Масштабный коэффициент для увеличения размера окна

// Размеры окна (вычисляются на основе размеров экрана и масштабного коэффициента)
int display_width = SCREEN_WIDTH * modifier;    // Ширина окна
int display_height = SCREEN_HEIGHT * modifier;   // Высота окна

// Цвета по умолчанию (в формате RGB: Красный, Зеленый, Синий)
float backgroundColor[3] = { 0.0f, 0.0f, 0.0f }; // Черный цвет фона
float pixelColor[3] = { 1.0f, 1.0f, 1.0f };       // Белый цвет пикселей

// Объявления функций (определения будут ниже)
void display();                     // Функция отрисовки (вызывается для обновления экрана)
void reshape_window(GLsizei w, GLsizei h); // Функция изменения размера окна
void keyboardUp(unsigned char key, int x, int y);   // Функция обработки отпускания клавиши
void keyboardDown(unsigned char key, int x, int y); // Функция обработки нажатия клавиши

// Использовать новый метод рисования (на основе текстур)
#define DRAWWITHTEXTURE 

typedef unsigned __int8 u8;          // Определяем тип u8 как unsigned char (8-битное целое без знака)
u8 screenData[SCREEN_HEIGHT][SCREEN_WIDTH][3]; // Массив для хранения данных текстуры (RGB)
void setupTexture();                // Функция настройки текстуры

int main(int argc, char** argv) {
    if (argc < 2) {                 // Если не указано имя файла ROM в аргументах командной строки
        printf("Usage: myChip8.exe chip8application\n\n"); // Выводим сообщение об использовании
        return 1;                   // Завершаем программу с кодом ошибки 1
    }

    // Загрузка ROM-файла (игры)
    if (!myChip8.loadApplication(argv[1])) { // Пытаемся загрузить приложение
        printf("Failed to load application.\n"); // Если загрузка не удалась
        return 1;                   // Завершаем программу с кодом ошибки 1
    }

    // Настройка OpenGL (инициализация GLUT)
    glutInit(&argc, argv);             // Инициализация GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // Двойная буферизация и RGBA-формат цвета

    glutInitWindowSize(display_width, display_height); // Установка размеров окна
    glutInitWindowPosition(320, 320);               // Установка позиции окна
    glutCreateWindow("myChip8");   // Создание окна с заголовком

    // Регистрация функций обратного вызова (callbacks)
    glutDisplayFunc(display);           // Функция отрисовки
    glutIdleFunc(display);              // Функция, вызываемая в режиме ожидания (для анимации)
    glutReshapeFunc(reshape_window);    // Функция изменения размера окна
    glutKeyboardFunc(keyboardDown);     // Функция обработки нажатия клавиши
    glutKeyboardUpFunc(keyboardUp);       // Функция обработки отпускания клавиши

#ifdef DRAWWITHTEXTURE
    setupTexture();                 // Настройка текстуры, если включена отрисовка с текстурами
#endif

    glutMainLoop();                     // Запуск основного цикла GLUT (обработка событий)

    return 0;                           // Завершение программы с кодом успеха 0
}

// Настройка текстуры (инициализация данных текстуры)
void setupTexture() {
    // Очистка данных экрана (заполнение нулями = черный цвет)
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
        for (int x = 0; x < SCREEN_WIDTH; ++x)
            screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;

    // Создание текстуры OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

    // Настройка параметров текстуры (фильтрация, обертывание)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Nearest-neighbor фильтрация
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Nearest-neighbor фильтрация
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);     // Запрет обертывания по горизонтали
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);     // Запрет обертывания по вертикали

    // Включение текстурирования
    glEnable(GL_TEXTURE_2D);
}

// Обновление текстуры (перенос данных из эмулятора на экран)
void updateTexture(const chip8& c8) {
    // Обновление пикселей в данных текстуры на основе состояния эмулятора
    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 64; ++x)
            if (c8.gfx[(y * 64) + x] == 0) { // Если пиксель выключен
                screenData[y][x][0] = static_cast<unsigned char>(backgroundColor[0] * 255);// Устанавливаем цвет фона
                screenData[y][x][1] = static_cast<unsigned char>(backgroundColor[1] * 255);
                screenData[y][x][2] = static_cast<unsigned char>(backgroundColor[2] * 255);
            }
            else {                            // Если пиксель включен
                screenData[y][x][0] = static_cast<unsigned char>(pixelColor[0] * 255);// Устанавливаем цвет пикселя
                screenData[y][x][1] = static_cast<unsigned char>(pixelColor[1] * 255);
                screenData[y][x][2] = static_cast<unsigned char>(pixelColor[2] * 255);
            }

    // Отправка обновленных данных в текстуру OpenGL
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

    // Установка цвета очистки экрана (фона)
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 0.0f);

    // Рисование квадрата, на который накладывается текстура
    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0); glVertex2d(0.0, 0.0); // Левый верхний угол
    glTexCoord2d(1.0, 0.0); glVertex2d(display_width, 0.0); // Правый верхний угол
    glTexCoord2d(1.0, 1.0); glVertex2d(display_width, display_height); // Правый нижний угол
    glTexCoord2d(0.0, 1.0); glVertex2d(0.0, display_height); // Левый нижний угол
    glEnd();
}

// Старый код графики (отрисовка квадратами)
void drawPixel(int x, int y) {
    glBegin(GL_QUADS);
    glColor3f(pixelColor[0], pixelColor[1], pixelColor[2]); // Установка цвета пикселя
    glVertex3f((static_cast<GLfloat>(x) * modifier) + 0.0f, (static_cast<GLfloat>(y) * modifier) + 0.0f, 0.0f);    // Левый верхний угол: определяет координату левого верхнего угла квадрата. x и y умножаются на modifier для масштабирования.
    glVertex3f((static_cast<GLfloat>(x) * modifier) + 0.0f, (static_cast<GLfloat>(y) * modifier) + modifier, 0.0f); // Левый нижний угол: определяет координату левого нижнего угла квадрата. y увеличивается на modifier, чтобы опуститься вниз.
    glVertex3f((static_cast<GLfloat>(x) * modifier) + modifier, (static_cast<GLfloat>(y) * modifier) + modifier, 0.0f); // Правый нижний угол: определяет координату правого нижнего угла квадрата. x и y увеличиваются на modifier.
    glVertex3f((static_cast<GLfloat>(x) * modifier) + modifier, (static_cast<GLfloat>(y) * modifier) + 0.0f, 0.0f);    // Правый верхний угол: определяет координату правого верхнего угла квадрата. x увеличивается на modifier.
    glEnd();
}

// Обновление экрана, рисуя квадраты для каждого пикселя
void updateQuads(const chip8& c8) {
    // Перебор всех пикселей экрана
    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 64; ++x) {
            if (c8.gfx[(y * 64) + x] == 0)  // Если пиксель выключен
                glColor3f(backgroundColor[0], backgroundColor[1], backgroundColor[2]); // Устанавливаем цвет фона
            else                               // Если пиксель включен
                glColor3f(pixelColor[0], pixelColor[1], pixelColor[2]); // Устанавливаем цвет пикселя

            drawPixel(x, y); // Рисуем квадрат для текущего пикселя
        }
}

const int cyclesPerFrame = 2; // Количество циклов Chip-8 за один кадр отрисовки

// Функция отрисовки (вызывается для обновления экрана)
void display() {
    static auto lastTime = std::chrono::high_resolution_clock::now(); // Сохраняем время последнего кадра (static для сохранения между вызовами)
    auto currentTime = std::chrono::high_resolution_clock::now(); // Получаем текущее время
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime); // Вычисляем время, прошедшее с последнего кадра

    const int targetFrameTime = 1000 / 60; // Целевое время для одного кадра (60 FPS)

    if (duration.count() >= targetFrameTime) { // Если прошло достаточно времени для отрисовки нового кадра
        lastTime = currentTime; // Обновляем время последнего кадра

        // Выполняем несколько циклов эмуляции Chip-8 за кадр
        for (int i = 0; i < cyclesPerFrame; ++i) {
            myChip8.emulateCycle(); // Эмулируем один цикл Chip-8
        }

        if (myChip8.drawFlag) { // Если Chip-8 запросил обновление экрана
            glClear(GL_COLOR_BUFFER_BIT); // Очищаем буфер кадра

#ifdef DRAWWITHTEXTURE
            updateTexture(myChip8); // Обновляем текстуру экрана
#else
            updateQuads(myChip8); // Обновляем экран, рисуя квадраты
#endif

            glutSwapBuffers(); // Меняем буферы (для двойной буферизации)
            myChip8.drawFlag = false; // Сбрасываем флаг отрисовки
        }
    }
}

// Функция изменения размера окна
void reshape_window(GLsizei w, GLsizei h) {
    // Установка цвета фона
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 0.0f);

    // Настройка матрицы проекции OpenGL
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0); // Ортографическая проекция (2D)
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);

    // Обновление размеров окна
    display_width = w;
    display_height = h;
}

// Функция обработки нажатия клавиши
void keyboardDown(unsigned char key, int x, int y) {
    printf("keyboardDown: key=%c\n", key);// Отладочный вывод
    // Модификация цветов (изменение цветовой палитры)
    if (key == 'i')
        pixelColor[0] += 0.1f; // Увеличиваем красную компоненту цвета пикселей
    else if (key == 'o')
        pixelColor[1] += 0.1f; // Увеличиваем зеленую компоненту цвета пикселей
    else if (key == 'p')
        pixelColor[2] += 0.1f; // Увеличиваем синюю компоненту цвета пикселей
    else if (key == 'I')
        backgroundColor[0] += 0.1f; // Увеличиваем красную компоненту цвета фона
    else if (key == 'O')
        backgroundColor[1] += 0.1f; // Увеличиваем зеленую компоненту цвета фона
    else if (key == 'P')
        backgroundColor[2] += 0.1f; // Увеличиваем синюю компоненту цвета фона

    // Поддержание значений цвета в диапазоне от 0.0 до 1.0
    for (int i = 0; i < 3; ++i) {
        if (pixelColor[i] > 1.0f)
            pixelColor[i] = 0.0f; // Если больше 1.0, обнуляем
        else if (pixelColor[i] < 0.0f)
            pixelColor[i] = 1.0f; // Если меньше 0.0, устанавливаем в 1.0
        if (backgroundColor[i] > 1.0f)
            backgroundColor[i] = 0.0f; // Если больше 1.0, обнуляем
        else if (backgroundColor[i] < 0.0f)
            backgroundColor[i] = 1.0f; // Если меньше 0.0, устанавливаем в 1.0
    }

    // Клавиши Chip-8 (привязка клавиш к кнопкам Chip-8)
    if (key == 27) // esc (выход)
        exit(0);   // Завершаем программу

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

// Функция обработки отпускания клавиши
void keyboardUp(unsigned char key, int x, int y)
{
    // Отпускание клавиш Chip-8 (сброс состояния кнопок)
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