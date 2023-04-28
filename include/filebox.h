#include <commdlg.h>

namespace wex
{

    /// A popup window where used can browse folders and select a file

    class filebox
    {
    public:
        filebox(gui &parent)
        {

            // Initialize OPENFILENAME
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = parent.handle();
            ofn.lpstrFile = szFile;
            // Set lpstrFile[0] to '\0' so that GetOpenFileName does not
            // use the contents of szFile to initialize itself.
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
        }

        void initDir(const std::string &dir)
        {
            ofn.lpstrInitialDir = dir.c_str();
        }
        void initFile(const std::string &fname)
        {
            ofn.lpstrFileTitle = (LPSTR)fname.c_str();
        }

        /** Set file filters

            @param[in] buf containing filter specifications

            example:

            "Log Files\0*.log\0All\0*.*\0"
        */
        void filter(
            const char *fbuf)
        {
            ofn.lpstrFilter = fbuf;
        }

        /// default extension for saved file, appended to whatever user enters
        void defaultExtension(const std::string &ext)
        {
            ofn.lpstrDefExt = ext.c_str();
        }

        // title for dialog window
        void title(const std::string &fname)
        {
            ofn.lpstrTitle = fname.c_str();
        }

        /** \brief prompt user for one file to open
            @return path to file to be opened, "" if cancelled
            If user selects multiple files, only the first will be returned
        */
        std::string open()
        {
            if (GetOpenFileNameA(&ofn) == TRUE)
            {
                myfname = ofn.lpstrFile;
                if (ofn.lpstrFile[ofn.nFileOffset - 1] == 0)
                {
                    // multiple files selected
                    // return the first
                    myfname += "\\";
                    myfname += &ofn.lpstrFile[ofn.nFileOffset];
                }
            }
            else
            {
                auto err = CommDlgExtendedError();
                myfname = "";
            }
            return myfname;
        }
        /**
         * @brief prompt user for one or multiple files
         * 
         * @return std::vector<std::string> filenames
         */
        std::vector<std::string> openMulti()
        {
            std::vector<std::string> ret;
            if (GetOpenFileNameA(&ofn) == TRUE)
            {
                if (ofn.lpstrFile[ofn.nFileOffset - 1] != 0)
                {
                    // single file selected
                    ret.push_back(ofn.lpstrFile);
                }
                else
                {
                    std::string dir = ofn.lpstrFile;
                    int p = ofn.nFileOffset;
                    while (1)
                    {
                        myfname = &ofn.lpstrFile[p];
                        if( myfname.empty())
                            break;
                        ret.push_back(dir + "\\" + myfname);
                        p += myfname.length() + 1;
                    }
                }
            }
            myfname = "";
            return ret;
        }
        /** \brief prompt user for folder and filename to save
            @return path to file to be saved, "" if cancelled
        */
        std::string save()
        {
            if (GetSaveFileNameA(&ofn) == TRUE)
                myfname = ofn.lpstrFile;
            else
                myfname = "";
            return myfname;
        }
        /// get filename entered by user
        std::string path() const
        {
            return myfname;
        }

    private:
        OPENFILENAME ofn; // common dialog box structure
        char szFile[260]; // buffer for file name
        std::string myfname;
    };
}
