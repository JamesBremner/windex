namespace wex
{

    /** @brief A read only table of values
     * 
     * The values are displayed in rows of columns.
     * The first column is assumed to be the row index.
     * The row index is returned when the row is clicked.
     * 
     * Usage:
     <pre>
         std::vector<std::vector<std::string>> table1data{
            {"1","a", "b", "c"},
            {"2","x", "y", "z"}};
        table &table1 = maker::make<table>(form);
        table1.move(10, 10, 350, 100);
        table1.set(table1data);
        form.events().asyncReadComplete(
        [&](int id)
        {
            std::cout <<"row " << id << "\n";
        });
    </pre>
     */

    class table : public gui
    {
    public:
        table(gui *parent)
            : gui(parent),
              myRowDisplayCount(25),
              myRowStart(0)
        {
            registerEventHandlers();
            text("");
        }

        /**
         * @brief Set table values
         * 
         * @param val 2D vector of values.  The inner vector contains the column value of one row
         */
        void set(const std::vector<std::vector<std::string>> &val)
        {
            myContents = val;
        }
        /**
         * @brief Set table values
         * 
         * @param val     1D vector of values, column by column
         * @param colcount count of columns in a row
         */
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
                    // if (col == 0)
                    //     vrow.push_back(
                    //         val[row * colcount + col] + " " + val[row * colcount + col + 1]);
                    // else if (col == 1)
                    //     continue;
                    // else
                        vrow.push_back(val[row * colcount + col]);
                }
                vvs.push_back(vrow);
            }
            set(vvs);
        }

        /// @brief Move display window
        /// @param i increment for row to display at top of window
        void rowInc(int i)
        {
            myRowStart += i;
            if (myRowStart < 0)
                myRowStart = 0;
        }

        void rowLastDisplay()
        {
            myRowStart = myContents.size() - 1;
        }

    private:
        std::vector<std::vector<std::string>> myContents;
        std::vector<int> myRowID;
        int myRowDisplayCount;
        int myRowStart;

        void draw(shapes &S)
        {
            myRowID.clear();

            if (!myContents.size())
                return;

            int colCount = myContents[0].size();
            int colWidth;
            if( colCount <= 1 )
                colWidth = size()[0];
            else
                colWidth = size()[0] / (colCount - 1);
            int rowStop;
            if (myContents.size() < 20)
            {
                myRowStart = 0;
                rowStop = myContents.size();
            }
            else
            {
                if (myRowStart > myContents.size() - 20)
                    myRowStart = myContents.size() - 20;
                rowStop = myRowStart + 30;
                if (rowStop > myContents.size())
                    rowStop = myContents.size();
            }
            // std::cout << "row " << myRowStart << " to " << myRowStart + rowStop - 1
            //           << " from " << myContents.size() << "\n";
            for (int kr = myRowStart; kr < rowStop; kr++)
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
                        // assumes first column contains the db id
                        myRowID.push_back(atoi(val.c_str()));
                    }
                    else
                    {
                        x = 50 + (kc - 1) * colWidth;
                        w = colWidth;
                    }
                    S.text(
                        val,
                        {x, (kr - myRowStart) * 20,
                         w - 5, 20});
                    S.line({x + w - 1, (kr - myRowStart) * 20,
                            x + w - 1, (kr - myRowStart + 1) * 20});
                }
            }
        }

        void registerEventHandlers()
        {
            events().draw(
                [&](PAINTSTRUCT &ps)
                {
                    wex::shapes S(ps);
                    draw(S);
                });

            events().click(
                [this]
                {
                    PostMessageA(
                        myParent->handle(),
                        wex::eventMsgID::asyncReadComplete,
                        myRowID[getMouseStatus().y / 20],
                        0);
                });
        }
    };
}
