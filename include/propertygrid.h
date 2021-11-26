#pragma once
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "wex.h"
namespace wex
{
    class propertyGrid;

    /// A name value pair
    class property
    {
    public:
        property(
            gui *parent,
            const std::string &name,
            const std::string &value)
            : myName(name), myValue(value), W(windex::get()),
              myLabel(wex::maker::make<wex::label>(*parent)),
              myEditbox(wex::maker::make<editbox>(*parent)),
              myCombobox(wex::maker::make<choice>(*parent)),
              myCheckbox(wex::maker::make<checkbox>(*parent)),
              myCategoryExpanded(wex::maker::make<checkbox>(*parent)),
              myLabelWidth(100),
              myType(eType::string),
              myLabelClicked(false)
        {
            myLabel.text(myName);
            myLabel.events().click(
                [this]
                {
                    myLabelClicked = true;
                });
            myLabel.events().clickPropogate();

            myEditbox.text(myValue);
        }
        property(
            gui *parent,
            const std::string &name,
            bool value)
            : myName(name), myValue(std::to_string((int)value)), W(windex::get()),
              myLabel(wex::maker::make<label>(*parent)), myEditbox(wex::maker::make<editbox>(*parent)), myCombobox(wex::maker::make<choice>(*parent)), myCheckbox(wex::maker::make<checkbox>(*parent)), myCategoryExpanded(wex::maker::make<checkbox>(*parent)), myLabelWidth(100), myType(eType::check)
        {
            myLabel.text(myName);
            myCheckbox.check(value);
            myCheckbox.text("");
            myCheckbox.events().clickPropogate();
        }

        // construct a choice property
        property(
            gui *parent,
            const std::string &name,
            const std::vector<std::string> &value)
            : myName(name), myValue(""), W(windex::get()), myLabel(wex::maker::make<label>(*parent)), myEditbox(wex::maker::make<editbox>(*parent)), myCombobox(wex::maker::make<choice>(*parent)), myCheckbox(wex::maker::make<checkbox>(*parent)), myCategoryExpanded(wex::maker::make<checkbox>(*parent)), myLabelWidth(100), myType(eType::choice)
        {
            myLabel.text(myName);
            myCombobox.itemHeight(30);
            for (auto &t : value)
            {
                myCombobox.add(t);
            }
        }
        /// construct a category
        property(
            gui *parent,
            const std::string &name)
            : myName(name), W(windex::get()), myLabel(wex::maker::make<label>(*parent)), myEditbox(wex::maker::make<editbox>(*parent)), myCombobox(wex::maker::make<choice>(*parent)), myCheckbox(wex::maker::make<checkbox>(*parent)), myCategoryExpanded(wex::maker::make<checkbox>(*parent)), myType(eType::category)
        {
            myCategoryExpanded.text(myName);
            myCategoryExpanded.plus();
            myCategoryExpanded.check();
            myCategoryExpanded.events().clickPropogate();
        }

        /// destructor
        ~property()
        {
            std::cout << myName << " delete\n";
            myLabel.~label();
            myEditbox.~editbox();
            myCategoryExpanded.~checkbox();
            myCombobox.~choice();
        }
        void move(const std::vector<int> &r)
        {
            std::vector<int> rl(r);
            rl[2] = myLabelWidth;
            myLabel.move(rl);

            // size of edit box
            std::vector<int> re(r);
            // right side of label
            re[0] += myLabelWidth;
            // window width minus label with minus scroll control
            re[2] -= myLabelWidth + 25;

            switch (myType)
            {
            case eType::string:
                myCategoryExpanded.move({0, 0, 0, 0});
                myEditbox.move(re);
                break;
            case eType::choice:
                re[3] *= myCombobox.count();
                myCombobox.move(re[0], re[1], re[2], re[3]);
                break;
            case eType::check:
                myCheckbox.move(re);
                break;
            case eType::category:
                re = r;
                re[1] += 0.5 * re[3];
                re[3] /= 2;
                myCategoryExpanded.move(re);
                myLabel.move({0, 0, 0, 0}); // keep unused label out of the way
                break;
            }
        }
        void labelWidth(int w)
        {
            myLabelWidth = w;
        }
        void bgcolor(int color)
        {
            myLabel.bgcolor(color);
        }

        bool labelClicked() const
        {
            return myLabelClicked;
        }
        void labelClicked(bool f)
        {
            myLabelClicked = f;
        }
        std::string labeltext() const
        {
            return myLabel.text();
        }

        void tabList(bool f = true)
        {
            if (myType != eType::string)
                return;
            auto s = GetWindowLongPtr(myEditbox.handle(), GWL_STYLE);
            if (f)
                s |= WS_TABSTOP;
            else
                s &= ~WS_TABSTOP;
            SetWindowLongPtr(
                myEditbox.handle(),
                GWL_STYLE,
                s);
        }

        /** Add pop help message when mouse hovers over property label
        @param[in] tip the help message
        @param[in] width of multiline tooltip, default single line
        @return property reference
    */
        property &tooltip(const std::string &tip, int width = 0)
        {
            myLabel.tooltip(tip, width);
            return *this;
        }
        /** Set property to be readonly
        @param[in] f true if property should be readonly, default true
        @return property reference

        A property defaults to editable when constructed.
    */
        property &readonly(bool f = true)
        {
            myCheckbox.enable(!f);
            switch (myType)
            {
            case eType::string:
                myEditbox.readonly(f);
                break;
            default:
                break;
            }
            return *this;
        }

        void show(bool f = true)
        {
            myLabel.show(f);
            switch (myType)
            {
            case eType::string:
                myEditbox.show(f);
                break;
            case eType::choice:
                myCombobox.show(f);
                break;
            case eType::check:
                myCheckbox.show(f);
                break;
            case eType::category:
                myLabel.show(false); // keep unused label out of the way
                myCategoryExpanded.show(f);
                break;
            }
        }
        /** Update ( redraw ) property child widgets

    Some widgets update nicely when the window containing the property grid resizes
    Others, most significantly labels, do not.  So application code should
    handle containing window resizes by calling propertyGrid::update()
    which will call this method on all properties.
    */
        void update()
        {
            myLabel.update();
            myCheckbox.update();
            myCategoryExpanded.update();
        }

        const std::string &name() const
        {
            return myName;
        }
        const std::string value() const
        {
            switch (myType)
            {
            case eType::string:
                return myEditbox.text();

            case eType::choice:
                return myCombobox.selectedText();

            case eType::check:
                return std::to_string((int)myCheckbox.isChecked());
                break;

            case eType::category:
                break;
            }
            return std::string("");
        }

        bool isChecked()
        {
            if (myType == eType::check)
                return myCheckbox.isChecked();
            return false;
        }
        /// set property value
        property &value(const std::string v)
        {
            switch (myType)
            {
            case eType::string:
                myValue = v;
                myEditbox.text(v);
                myEditbox.update();
                break;
            case eType::choice:
                if (v.empty())
                {
                    myCombobox.select(-1);
                    break;
                }
                myValue = v;
                myCombobox.select(v);
                break;
            default:
                break;
            }
            return *this;
        }
        void value_bool(bool v)
        {
            switch (myType)
            {
            case eType::check:
                myValue = std::to_string((int)v);
                myCheckbox.check(v);
                break;
            default:
                // other property types ignore requests to change boolean vaue
                break;
            }
        }
        /// copy value from gui into myValue attribute
        void saveValue()
        {
            switch (myType)
            {
            case eType::string:
                myValue = myEditbox.text();
                break;
            case eType::choice:
                myValue = myCombobox.selectedText();
                break;
            case eType::check:
                myValue = std::to_string((int)myCheckbox.isChecked());
                break;
            case eType::category:
                break;
            }
        }

        // get myValue attribute
        const std::string savedValue() const
        {
            return myValue;
        }

        bool isCategory() const
        {
            return myType == eType::category;
        }
        bool isExpanded() const
        {
            if (!isCategory())
                return false;
            return myCategoryExpanded.isChecked();
        }
        void expand(bool f)
        {
            myCategoryExpanded.check(f);
        }
        /// register function to call when property value changes
        void change(std::function<void()> f)
        {
            switch (myType)
            {
            case eType::string:
                myEditbox.events().change(myEditbox.id(), f);
                break;
            case eType::check:
                myCheckbox.events().click(f);
                break;
            default:
                break;
            }
        }

        void BoostPropertyTree(
            boost::property_tree::ptree &tree,
            const std::string &catname)
            const
        {
            if (myType == eType::choice)
                tree.put(
                    catname + myName + ".value",
                    myCombobox.selectedText());
            else
                tree.put(
                    catname + myName + ".value",
                    myValue);
            tree.put(
                catname + myName + ".type",
                std::to_string((int)myType));
            if (myType == eType::choice)
            {
                for (int k = 0; k < myCombobox.count(); k++)
                    tree.put(
                        catname + myName + ".choice" + std::to_string(k),
                        myCombobox.text(k));
            }
        }

    private:
        std::string myName;
        std::string myValue;
        wex::windex &W;
        wex::label &myLabel;
        wex::editbox &myEditbox;
        wex::choice &myCombobox;
        wex::checkbox &myCheckbox;
        wex::checkbox &myCategoryExpanded;
        int myLabelWidth;
        enum class eType
        {
            string,
            choice,
            check,
            category
        } myType;
        bool myLabelClicked;
    };
    /** A grid of properties.

Add boost to the compiler include search list
*/
    class propertyGrid : public gui
    {
    public:
        typedef std::shared_ptr<property> prop_t;

        propertyGrid(gui *parent)
            : gui(parent, "windex", WS_CHILD, WS_EX_CONTROLPARENT), myHeight(25), myHeightCategory(2), myWidth(300), myLabelWidth(100), myBGColor(0xc8c8c8), myfScroll(false), myftabstop(false)
        {
            text("PG");

            // handle click event
            events().click(
                [this]
                {
                    labelClick();
                    visible();
                });

            // regsiter NOP event handlers
            change([] {});
            nameClick([](const std::string &) {});
        }
        void clear()
        {
            myProperty.clear();
        }
        /** Add string property
        @param[in] name of property
        @param[in] value initial value
        @return reference to property
    */
        property &string(
            const std::string &name,
            const std::string &value)
        {
            myProperty.push_back(prop_t(new property(this, name, value)));
            CommonConstruction();
            return *myProperty.back().get();
        }
        /** Add choice property
        @param[in] name of property
        @param[in] choice vector of choices to be selected from
        @return reference to property
    */
        property &choice(
            const std::string &name,
            const std::vector<std::string> &choice)
        {
            myProperty.push_back(prop_t(new property(this, name, choice)));
            CommonConstruction();
            return *myProperty.back().get();
        }
        /** Add boolean property
        @param[in] name of property
        @param[in] f default
        @return reference to property
    */
        property &check(
            const std::string &name,
            bool f)
        {
            myProperty.push_back(prop_t(new property(this, name, f)));
            CommonConstruction();
            return *myProperty.back().get();
        }
        /// Add categoty
        void category(
            const std::string &name)
        {
            myProperty.push_back(prop_t(new property(this, name)));
            CommonConstruction();
        }

        /** Add properties from boost property tree
        @param[in] pt property tree

        Top level becomes categories
        Second level becomes properties
    */
        void add(boost::property_tree::ptree &pt)
        {
            // loop over categories
            for (auto cat : pt)
            {
                category(cat.first);

                // loop over properties in category
                for (auto prop : cat.second)
                {
                    int type = pt.get<int>(cat.first + "." + prop.first + "." + "type", 0);

                    switch (type)
                    {
                    case 1:
                    {
                        // choice property

                        std::vector<std::string> vc;
                        for (auto ch : prop.second)
                        {
                            if (ch.first.find("choice") == 0)
                            {
                                vc.push_back(ch.second.data());
                            }
                        }
                        choice(prop.first, vc).value(pt.get<std::string>(cat.first + "." + prop.first + "." + "value"));
                    }
                    break;

                    default:

                        // string property

                        string(
                            prop.first,
                            pt.get<std::string>(cat.first + "." + prop.first + "." + "value", "missing"));
                        break;
                    }
                }
            }
        }

        /// Get properties as a boost property tree
        boost::property_tree::ptree
        BoostPropertyTree()
        {
            // ensure that values user sees are stored in value attributes
            saveValues();

            boost::property_tree::ptree tree;
            std::string catname;
            for (auto p : myProperty)
            {
                if (p->isCategory())
                {
                    catname = p->name() + ".";
                }
                else
                {
                    p->BoostPropertyTree(tree, catname);
                }
            }
            return tree;
        }

        /// Get properties as a JSON string
        std::string json()
        {
            std::stringstream ss;
            write_json(ss, BoostPropertyTree());
            return ss.str();
        }

        /** Add properties for a JSON string
        @param[in] json string

        Throws exception if format error in JSON
    */
        void addjson(const std::string &json)
        {
            std::cout << "->addjson\n";
            std::stringstream ss(json);
            boost::property_tree::ptree tree;
            read_json(ss, tree);
            std::cout << "<-read\n";
            add(tree);
            std::cout << "<-addjson\n";
        }

        /// Add vertical scrollbar
        void scroll()
        {
            myfScroll = true;
            gui::scroll(false);
        }
        /// Expand, or contract, category of properties
        void expand(
            const std::string name,
            bool fexpand = true)
        {
            for (auto &p : myProperty)
            {
                if (p->name() == name &&
                    p->isCategory())
                {
                    p->expand(fexpand);
                    visible();
                    return;
                }
            }
        }
        void expandAll(
            bool fexpand = true)
        {
            for (auto &p : myProperty)
            {
                if (p->isCategory())
                {
                    p->expand(fexpand);
                }
            }
            visible();
        }
        void move(const std::vector<int> &r)
        {
            gui::move(r);
            myWidth = r[2];
        }
        void labelWidth(int w)
        {
            myLabelWidth = w;
        }

        /// force every property to redraw its label
        void update()
        {
            for (auto &p : myProperty)
                p->update();
            gui::update();
        }

        /// get pointer to first property with name, ignoring categories
        property *find(const std::string &name)
        {
            for (auto &p : myProperty)
            {
                //std::cout << "property::find " << name <<" "<< p.name() << "\n";
                if (p->name() == name)
                    return p.get();
            }
            return nullptr;
        }

        /// get pointer to first property with name in a category
        property *find(
            const std::string &category,
            const std::string &name)
        {
            for (auto p = myProperty.begin();
                 p != myProperty.end();
                 p++)
            {
                if ((*p)->isCategory())
                {
                    if ((*p)->name() == category)
                    {
                        for (p++;
                             p != myProperty.end();
                             p++)
                        {
                            if ((*p)->isCategory())
                                return nullptr;
                            if ((*p)->name() == name)
                                return p->get();
                        }
                    }
                }
            }
            return nullptr;
        }

        /// get value in editbox of property with name
        const std::string value(const std::string &name)
        {
            std::cout << "PG value " << name << "\n";
            property *p = find(name);
            if (!p)
            {
                static std::string null;
                return null;
            }
            std::string v = p->value();
            return p->value();
        }

        bool isChecked(const std::string &name)
        {
            property *p = find(name);
            if (!p)
                return false;
            return p->isChecked();
        }

        /// save values in all property textboxes in the property's myValue attribute
        void saveValues()
        {
            for (auto &p : myProperty)
            {
                p->saveValue();
            }
        }

        int propHeight() const
        {
            return myHeight;
        }

        /// set property display height in pixels.  Default 25
        void propHeight(int h)
        {
            myHeight = h;
        }

        /// set category height low i.e. just one property height, rather than 2x prop height
        void categoryHeightLow(bool f = true)
        {
            if (f)
                myHeightCategory = 1;
            else
                myHeightCategory = 2;
        }
        int width() const
        {
            return myWidth;
        }
        int propCount() const
        {
            return (int)myProperty.size();
        }
        /// Register function to call when property value has changed
        void change(std::function<void()> f)
        {
            onChange = f;
        }
        /// Register function to call when property name is clicked
        void nameClick(std::function<void(const std::string &)> f)
        {
            onName = f;
        }

        /** Enable tab stepping through the properties

    Note: all windows need to have WS_EX_CONTROLPARENT style
    otherwise the message pump hangs  https://stackoverflow.com/a/11090609/16582
    */
        void tabList(bool f = true)
        {
            // set flag so that any properties added later will have the tabstop style
            myftabstop = f;

            // add tabstop style to existing properties
            for (auto p : myProperty)
                p->tabList(f);
        }

    private:
        std::vector<prop_t> myProperty; // the properties in the grid
        int myHeight;                   // height of a single property
        int myHeightCategory;
        int myWidth;      // width of grid
        int myLabelWidth; // width of property labels
        int myBGColor;    // grid background color
        bool myfScroll;   // true if scrollbars used
        bool myftabstop;
        std::function<void()> onChange;                  // funtion to call when property has changed
        std::function<void(const std::string &)> onName; // functio to call when property name is clicked

        void CommonConstruction()
        {
            prop_t P = myProperty.back();
            P->labelWidth(myLabelWidth);
            P->bgcolor(myBGColor);
            if (myfScroll)
            {
                //std::cout << "pg scroll range " << ((int)myProperty.size()+1) * myHeight << "\n";
                scrollRange(
                    myWidth,
                    ((int)myProperty.size() + 1) * myHeight);
            }

            visible();

            P->change([this]
                      { onChange(); });

            if (myftabstop)
                P->tabList();
        }
        void labelClick()
        {
            // search properties for the one whose label was clicked
            for (auto P : myProperty)
            {
                if (P->labelClicked())
                {

                    // run regsitered event handler
                    onName(P->labeltext());

                    // clear the clicked flag
                    P->labelClicked(false);
                }
            }
        }
        /** Show properties when category is expanded or collapsed
    */
        void visible()
        {
            // hide all the properties
            for (auto P : myProperty)
                P->show(false);

            // show properties in expanded categories
            bool expanded = true; /// true if current category is expanded
            int index = 0;        /// number of property heights displayed so far
            for (auto P : myProperty)
            {
                if (P->isCategory())
                {
                    //std::cout << "cat " << P.name()  << " at " << index << "\n";
                    P->move({0, index * myHeight, myWidth, myHeightCategory * myHeight}); // category always visible
                    P->show();
                    index += myHeightCategory;  // display takes two rows
                    expanded = P->isExpanded(); // control visibility of contained properties
                }
                else if (expanded)
                {
                    //std::cout << "show " << P.name() <<" at "<< index << "\n";
                    P->move({0, index * myHeight, myWidth, myHeight}); // property is visible
                    P->show();
                    index++; // displays in one row
                }
                else
                {
                    //std::cout << "hide " << P.name() << "\n";
                    P->move({0, 0, 0, 0}); // invisible property
                    P->show(false);
                }
            }
            if (myfScroll)
                scrollRange(
                    myWidth,
                    index * myHeight);
            update();
        }
    };
}
