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
        wcout << ++c << L'\t' << line << L'\n';

        if (line[0] == '*') {
            jabas.push_back(info);
            r = 0;
        }
        else if (line[0] == '&') {
            secondary = true;
            continue;
        }
        switch(r) {
        case 0:     info.name = line;               break;
        case 1:     info.tg = line;                 break;
        case 2:     info.irc = line;                break;
        case 3:     info.imageFileName = line;      break;
        default:
        if (secondary)
            info.secondaryNakkis.push_back(line);
        else
            info.mainNakkis.push_back(line);
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
        while (text.getLocalBounds().height > fitInto) {
            printf("%0.2f\n",text.getLocalBounds().height);
            text.setCharacterSize(--s);
        }

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

    bool shrinkSecondary = nSpaces < info.secondaryNakkis.size();

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

int main(void) {
    std::vector<JabaInfo> jabas;
    readJabas(jabas, "jabat.txt");

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
