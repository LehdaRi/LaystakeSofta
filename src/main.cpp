#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <codecvt>
#include <fcntl.h>
#include <io.h>


using namespace std;


struct JabaInfo {
    wstring name;
    wstring tg;
    wstring irc;
    wstring imageFileName;
    std::vector<wstring> nakkis;
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

    wifs.imbue(locale(wifs.getloc(), new codecvt_utf8<wchar_t, 0x10ffff, consume_header>()));
    while(getline(wifs, line)) {
        wcout << ++c << L'\t' << line << L'\n';

        if (line[0] == '*') {
            jabas.push_back(info);
            r = 0;
        }
        switch(r) {
        case 0:     info.name = line;               break;
        case 1:     info.tg = line;                 break;
        case 2:     info.irc = line;                break;
        case 3:     info.imageFileName = line;      break;
        default:    info.nakkis.push_back(line);    break;
        }
        ++r;
    }
    wcout << endl;
}


void renderString(sf::RenderTexture& tex, sf::Font& font, const wstring& str, int x, int y) {
    sf::Glyph glyph;
    sf::Texture glyphTexture;

    sf::Text text;
    text.setFont(font);
    text.setString(sf::String(str));
    text.setCharacterSize(100);
    text.setColor(sf::Color::White);
    text.setPosition(x, y);
    tex.draw(text);
}


void render(sf::RenderTexture& tex, sf::Font& font, const JabaInfo& info) {
    const int baseX = 1770;
    int charX = baseX;
    int charY = 572;

    /*renderString(tex, font, L"---------------------", charX, charY);
    charY += 113; charX = baseX;
    renderString(tex, font, info.name, charX, charY);
    charY += 113; charX = baseX;
    renderString(tex, font, L"---------------------", charX, charY);
    charY += 113; charX = baseX;

    for (auto& nakki : info.nakkis) {
        renderString(tex, font, nakki, charX, charY);
        charY += 113; charX = baseX;
    }

    charY = 1617; charX = baseX;
    renderString(tex, font, L"---------------------", charX, charY);
    charY += 113; charX = baseX;

    wstring tg;
    tg = L"TG: @" + info.tg;
    renderString(tex, font, info.tg, charX, charY);
    charY += 113; charX = baseX;

    wstring irc;
    tg = L"IRC: " + info.irc;
    renderString(tex, font, info.irc, charX, charY);*/


    wstring str;
    str += L"---------------------\n";
    str += info.name + L"\n";
    str += L"---------------------\n";

    for (auto& nakki : info.nakkis)
        str += nakki + L"\n";

    str += L"---------------------\n";
    if (info.tg.size() > 0)
        str += L"TG: @" + info.tg;
    str += L"\n";
    if (info.irc.size() > 0)
        str += L"IRC: " + info.irc + L"\n";

    renderString(tex, font, str, charX, charY);
}

int main(void) {
    std::vector<JabaInfo> jabas;
    readJabas(jabas, "test.txt");

    sf::Font font;
    font.loadFromFile("courbd.ttf");

    sf::RenderTexture destTex;
    destTex.create(3300, 2100);
    destTex.clear(sf::Color::Black);
    render(destTex, font, jabas[0]);

    sf::Image destImg;
    destImg = destTex.getTexture().copyToImage();
    destImg.flipVertically();
    destImg.saveToFile("test.png");

    return 0;
}
