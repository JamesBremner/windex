namespace wex {

/// @brief A read only table of values

class table
{
public:
    table(gui &parent)
        : myPanel(maker::make()),
          myParent(&parent),
          myftracking(false),
          myRowDisplayCount(35),
          myRowStart(0)
    {
        myPanel.bgcolor(0xFFFFFF);

        registerEventHandlers();
    }

    void move(int x, int y, int w, int h)
    {
        myPanel.move(x, y, w, h);
        // myPanel.scrollRange(1500, 1600);
    }

    void set(const std::vector<std::vector<std::string>> &val)
    {
        myContents = val;
    }
    void set(const std::vector<std::string> &val, int colcount)
    {
        std::vector<std::vector<std::string>> vvs;
        for (
            int row = 0;
            row < val.size() / colcount;
            row++)
        {
            std::vector<std::string> vrow;
            for (int col = 0; col < colcount; col++)
            {
                if (col == 1)
                    vrow.push_back(
                        val[row * colcount + col] + " " + val[row * colcount + col + 1]);
                else if (col == 2)
                    continue;
                else
                    vrow.push_back(val[row * colcount + col]);
            }
            vvs.push_back(vrow);
        }
        set(vvs);
    }

    void show(bool f = true)
    {
        if (f)
        {
            myPanel.move(10, 10, 1500, 800);
            myPanel.show();
            // SetCapture(myPanel.handle());
        }
        else
        {
            myPanel.show(false);
            ReleaseCapture();
        }
    }

private:
    gui &myPanel;
    gui *myParent;
    int myColCount;
    std::vector<std::vector<std::string>> myContents;
    bool myftracking;
    std::vector<int> myRowID;
    int myRowDisplayCount;
    int myRowStart;

    void draw(shapes &S)
    {
        if (!myContents.size())
            return;

        int colCount = myContents[0].size();
        int colWidth = myPanel.size()[0] / colCount;
        // int rowCount = myRowDisplayCount;
        // if (myRowStart + rowCount > myContents.size())
        //     rowCount = myContents.size() - myRowStart;
        // std::cout << "row " << myRowStart << " to " << myRowStart + rowCount - 1
        //           << " from " << myContents.size() << "\n";
        for (int kr = myRowStart; kr < myContents.size(); kr++)
        // for (int kr = myRowStart; kr <25; kr++)
        {
            if (myContents[kr].size() > colCount)
                throw std::runtime_error(
                    "bad col count");

            for (int kc = 0; kc < colCount; kc++)
            {
                auto &val = myContents[kr][kc];
                int x, w;
                if (kc == 0)
                {
                    x = 0;
                    w = 50;

                    // save the database ID of the row
                    // so it can be sent if the row is double clicked
                    myRowID.push_back(atoi(val.c_str()));
                }
                else
                {
                    x = 50 + (kc - 1) * colWidth;
                    w = colWidth + 5;
                }
                S.text(
                    val,
                    {x, (kr - myRowStart) * 20,
                     w, 20});
            }
        }
    }

        void registerEventHandlers()
        {
            myPanel.events().draw(
                [&](PAINTSTRUCT &ps)
                {
                    wex::shapes S(ps);
                    draw(S);
                });

            myPanel.events().mouseMove(
                [this](wex::sMouse &m)
                {
                    // std::cout << "move " << myftracking << "\n";
                    if (myftracking)
                        return;

                    // first mouse movement in slider
                    // start tracking mouse movement
                    myftracking = true;

                    // generate event when mouse leaves
                    TRACKMOUSEEVENT s;
                    s.cbSize = sizeof(s);
                    s.hwndTrack = myPanel.handle();
                    s.dwFlags = TME_LEAVE;
                    TrackMouseEvent(&s);

                    // SetCapture(myPanel.handle());

                    // run mouse enter event handler
                    myPanel.events().onMouseEnter();
                });
            myPanel.events().mouseEnter(
                [this]
                {
                    // std::cout << "table mouse enter\n";
                });
            myPanel.events().mouseLeave(
                [this]
                {
                    // std::cout << "table mouse left\n";
                    myftracking = false;
                    ReleaseCapture();
                });
            myPanel.events().clickDouble(
                [this]
                {
                    int row = myPanel.getMouseStatus().y / 20
                        + myRowStart;
                    int id = -1;
                    if (row < myRowID.size())
                        id = myRowID[row];
                    // std::cout << "DClick " << id << "\n";
                    PostMessageA(
                        myParent->handle(),
                        wex::eventMsgID::asyncReadComplete,
                        id,
                        0);
                });
            myPanel.events().keydown(
                [this](int c)
                {
                    std::cout << "keydown " << c << "\n";
                    switch (c)
                    {
                    case 40:
                        myRowStart += myRowDisplayCount;
                        break;
                    case 38:
                        myRowStart -= myRowDisplayCount;
                        if (myRowStart < 0)
                            myRowStart = 0;
                        break;
                    default:
                        return;
                    }
                    myPanel.update();
                });
        }
    };
}
