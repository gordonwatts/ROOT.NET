using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace TestUtilities
{
    public class FileUtilities
    {
        public static bool FindString (string filename, string searches)
        {
            FileInfo info = new FileInfo(filename);
            if (!info.Exists)
            {
                return false;
            }

            var f = info.OpenText();
            string allofit = f.ReadToEnd();
            return allofit.IndexOf(searches) >= 0;
        }
    }
}
