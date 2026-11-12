#include "graphics2d/FontRenderer.h"
#include <GL/freeglut.h>

static void* getGlutFont(FontRenderer::FontSize size) {
    switch (size) {
        case FontRenderer::SMALL:  return GLUT_BITMAP_HELVETICA_10;
        case FontRenderer::NORMAL: return GLUT_BITMAP_9_BY_15;
        case FontRenderer::LARGE:  return GLUT_BITMAP_HELVETICA_18;
    }
    return GLUT_BITMAP_9_BY_15;
}

void FontRenderer::drawText2D(int x, int y, const std::string& text, const Vec3& color, FontSize size) {
    glColor3f(color.x, color.y, color.z);
    glRasterPos2i(x, y);
    void* font = getGlutFont(size);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

void FontRenderer::drawText2DWithShadow(int x, int y, const std::string& text, const Vec3& color, FontSize size) {
    // Lớp đổ bóng màu đen mờ lệch 1 pixel
    drawText2D(x + 1, y - 1, text, Vec3(0.05f, 0.05f, 0.05f), size);
    // Lớp chữ chính
    drawText2D(x, y, text, color, size);
}

void FontRenderer::drawText3D(const Vec3& worldPos, const std::string& text, const Vec3& color) {
    glDisable(GL_LIGHTING);
    glColor3f(color.x, color.y, color.z);
    glRasterPos3f(worldPos.x, worldPos.y, worldPos.z);
    void* font = GLUT_BITMAP_8_BY_13;
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

int FontRenderer::getTextWidth(const std::string& text, FontSize size) {
    void* font = getGlutFont(size);
    int width = 0;
    for (char c : text) {
        width += glutBitmapWidth(font, c);
    }
    return width;
}
