#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <locale>
#include <codecvt>
#include <fcntl.h>
#include <io.h>


#define BLEND(C1, C2, B) sf::Color(C1.r+B*(C2.r-C1.r), C1.g+B*(C2.g-C1.g), C1.b+B*(C2.b-C1.b), C1.a+B*(C2.a-C1.a))


using namespace std;


struct JabaInfo {
    wstring name;
    wstring tg;
    wstring irc;
    wstring imageFileName;
    std::vector<wstring> mainNakkis;
    std::vector<wstring> secondaryNakkis;
};


void readJabas(std::vector<JabaInfo>& jabas, const std::string& fileName)
{
    wifstream wifs;
    wstring line;
    int c = 0;

    wifs.open(fileName);
    if(!wifs.is_open())
    {
        wcerr << L"Unable to open file" << endl;
        return;
    }

    JabaInfo info;
    int r = 0;
    bool secondary = false;

    wifs.imbue(locale(wifs.getloc(), new codecvt_utf8<wchar_t, 0x10ffff, consume_header>()));
    while(getline(wifs, line)) {
        //wcout << ++c << L'\t' << line << L'\n';

        if (line[0] == '*') {
            jabas.push_back(info);
            info.mainNakkis.clear();
            info.secondaryNakkis.clear();
            r = 0;
            secondary = false;
            cout << "" << endl;
            continue;
        }
        else if (line[0] == '&') {
            secondary = true;
            wcout << "Secondary Nakkis:" << endl;
            continue;
        }
        switch(r) {
        case 0:
            info.name = line;
            wcout << info.name << endl;
        break;
        case 1:
            info.tg = line;
            wcout << "TG: @" << info.tg << endl;
        break;
        case 2:
            info.irc = line;
            wcout << "IRC: " << info.irc << endl;
        break;
        case 3:
            info.imageFileName = L"res/" + line;
            wcout << "Main Nakkis:" << endl;
        break;
        default:
        if (secondary)
            info.secondaryNakkis.push_back(line);
        else
            info.mainNakkis.push_back(line);

        wcout << "  " << line << endl;
        break;
        }
        ++r;
    }
    wcout << endl;
}


void renderString(sf::RenderTexture& tex, sf::Font& font, const wstring& str, int x, int y, int fitInto = 0) {
    if (fitInto) {
        sf::Text text;
        text.setFont(font);
        text.setString(str);

        int s = 100;
        text.setCharacterSize(s);
        while (text.getLocalBounds().height > fitInto)
            text.setCharacterSize(--s);

        text.setString(sf::String(str));
        text.setColor(sf::Color::White);
        text.setPosition(x, y);
        tex.draw(text);
    }
    else {
        sf::Text text;
        text.setFont(font);
        text.setString(sf::String(str));
        text.setCharacterSize(100);
        text.setColor(sf::Color::White);
        text.setPosition(x, y);
        tex.draw(text);
    }
}


void render(sf::RenderTexture& tex, sf::Font& font, const JabaInfo& info) {
    int charX = 1770;
    int charY = 572;

    wstring str;
    str += L"---------------------\n";
    str += info.name + L"\n";
    str += L"---------------------\n";

    for (auto& nakki : info.mainNakkis)
        str += nakki + L"\n";

    int nSpaces = 5 - info.mainNakkis.size();
    if (info.tg.size() == 0)
        ++nSpaces;
    if (info.irc.size() == 0)
        ++nSpaces;

    bool shrinkSecondary = (unsigned)nSpaces < info.secondaryNakkis.size();

    if (shrinkSecondary) {
        for (int i=0; i<nSpaces; ++i)
            str += L"\n";
    }
    else {
        for (auto& nakki : info.secondaryNakkis)
            str += nakki + L"\n";
    }

    str += L"---------------------\n";
    if (info.tg.size() > 0)
        str += L"TG: @" + info.tg;
    str += L"\n";
    if (info.irc.size() > 0)
        str += L"IRC: " + info.irc + L"\n";

    renderString(tex, font, str, charX, charY);

    str.clear();
    //printf("%i", nSpaces*113);

    if (shrinkSecondary && info.secondaryNakkis.size() > 0) {
        for (auto& nakki : info.secondaryNakkis)
            str += nakki + L"\n";
        renderString(tex, font, str, charX, 572+113*(info.mainNakkis.size()+3), nSpaces*113+55);
    }
}

sf::Image resizeImage(sf::Image& srcImg, float width, float height) {
    sf::Image image;
    image.create(width, height);
    auto s = srcImg.getSize();
    float xs = s.x/width;
    float ys = s.y/height;

    for (auto y=0u; y<(unsigned)height; ++y) {
        for (auto x=0u; x<(unsigned)width; ++x) {
            float xd = x*xs;
            float yd = y*ys;
            int xi = (int)xd;
            int yi = (int)yd;
            int xi2 = xi+1;
            if (xi2 >= s.x) xi2 = xi;
            int yi2 = yi+1;
            if (yi2 >= s.y) yi2 = yi;
            xd -= xi;
            yd -= yi;

            auto p00 = srcImg.getPixel(xi, yi);
            auto p10 = srcImg.getPixel(xi2, yi);
            auto p01 = srcImg.getPixel(xi, yi2);
            auto p11 = srcImg.getPixel(xi2, yi2);

            auto p0 = BLEND(p00, p10, xd);
            auto p1 = BLEND(p01, p11, xd);

            image.setPixel(x, y, BLEND(p0, p1, yd));
        }
    }

    printf("%f %f\n", xs, ys);
    return image;
}

int main(void) {
    std::vector<JabaInfo> jabas;
    readJabas(jabas, "res/jabat.txt");

    sf::Font font;
    font.loadFromFile("courbd.ttf");

    sf::Image borderImg;
    borderImg.loadFromFile("border.png");
    sf::Image crtmaskImg;
    crtmaskImg.loadFromFile("crtmask.png");

    sf::Image combinedImg;
    combinedImg.create(6600, 4200);
    int combinedId = 0;
    int laystakeId=0;

    for (auto& jaba : jabas) {
        sf::RenderTexture destTex;
        destTex.create(3300, 2100);
        destTex.clear(sf::Color::Black);
        render(destTex, font, jaba);

        sf::Image destImg;
        destImg = destTex.getTexture().copyToImage();
        destImg.flipVertically();

        sf::Image jabaImg;
        jabaImg.loadFromFile(sf::String(jaba.imageFileName));
        auto s = jabaImg.getSize();

        float jr = (float)s.x/(float)s.y;
        int jabaSizeX = 0, jabaSizeY = 0;
        if (jr >= 1100.0f/1600.0f) {
            jabaImg = resizeImage(jabaImg, 1600.0f*jr, 1600.0);
            jabaSizeX = 1600.0f*jr;
            jabaSizeY = 1600;
        }
        else {
            jabaImg = resizeImage(jabaImg, 1100.0f, 1100.0f/jr);
            jabaSizeX = 1100.0f;
            jabaSizeY = 1100.0f/jr;
        }

        for (auto y=0; y<2100; ++y) {
            for (auto x=0; x<3300; ++x) {
                auto p = destImg.getPixel(x, y);
                auto pBorder = borderImg.getPixel(x, y);
                auto pCrtmask = crtmaskImg.getPixel(x, y);
                auto jabaX = x-800+(jabaSizeX/2);
                auto jabaY = y-250;
                sf::Color pJaba = sf::Color::Black;
                if (jabaY >= 0 && jabaY < jabaSizeY && jabaX >= 0 && jabaX < jabaSizeX)
                    pJaba = jabaImg.getPixel(jabaX, jabaY);

                if (pBorder.a > 0)
                    destImg.setPixel(x, y, BLEND(pJaba, ((pBorder + p)*pCrtmask), (pBorder.a/255.0f)));
                else
                    destImg.setPixel(x, y, pJaba);
            }
        }

        std::basic_stringstream<wchar_t> ss;
        ss << "laystakkeet/" << jaba.name << L".png";
        destImg.saveToFile(sf::String(ss.str()));

        int xx;
        int yy;
        switch (laystakeId++) {
        case 0:
        {
            xx = 0;
            yy = 0;
            if (combinedId++ > 0) {
                stringstream ss;
                ss << "laystakkeet/laystakkeet" << combinedId-1 << ".png";
                combinedImg.saveToFile(ss.str());
            }
        }
        break;
        case 1:
            xx = 3300;
            yy = 0;
        break;
        case 2:
            xx = 0;
            yy = 2100;
        break;
        case 3:
            xx = 3300;
            yy = 2100;
            laystakeId = 0;
        break;
        }

        for (auto jj=0; jj<2100; ++jj) {
            for (auto ii=0; ii<3300; ++ii) {
                combinedImg.setPixel(ii+xx, jj+yy, destImg.getPixel(ii, jj));
            }
        }
    }

    stringstream ss;
    ss << "laystakkeet/laystakkeet" << combinedId << ".png";
    combinedImg.saveToFile(ss.str());

    return 0;
}
