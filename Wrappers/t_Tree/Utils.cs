using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using ROOTNET;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;

namespace t_Tree
{
    static class Utils
    {
        /// <summary>
        /// Open and get an ntuple
        /// </summary>
        /// <param name="rootFileName"></param>
        /// <param name="treeName"></param>
        /// <returns></returns>
        public static NTTree OpenAndGet(string rootFileName, string treeName)
        {
            Assert.IsTrue(File.Exists(rootFileName), string.Format("File {0} can't be found", rootFileName));
            var f = NTFile.Open(rootFileName, "READ");
            Assert.IsTrue(f.IsOpen(), "File open");
            var to = f.Get(treeName);
            Assert.IsNotNull(to, string.Format("{0} not in the file", treeName));
            var t = to as ROOTNET.NTTree;
            Assert.IsNotNull(t, string.Format("{0} is not a TTree object", treeName));
            return t;
        }
    }
}
