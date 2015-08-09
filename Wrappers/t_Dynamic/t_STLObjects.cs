using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace t_Dynamic
{
    [TestClass]
    public class t_STLObjects
    {
        // Force a reference to the monolithic wrapper guys - so that we can
        // make sure that the wrappers get loaded into memory.
#pragma warning disable 0414
        static ROOTNET.NTObject _obj = null;
#pragma warning restore 0414

        [TestInitialize]
        public void TestInit()
        {
            ROOTNET.Utility.ROOTCreator.ResetROOTCreator();
        }

        [TestMethod]
        public void TestVectorCTor()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("vector<int>");
        }

        [TestMethod]
        public void TestVectorCTorFullSpec()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("vector<int,allocator<int> >");
        }

        [TestMethod]
        public void TestVectorCTorAccess()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("vector<int,allocator<int> >");
            Assert.AreEqual((uint) 0, vec.size(), "Size");
            vec.push_back(5);
            Assert.AreEqual((uint) 1, vec.size(), "size after push_back");
        }

        [TestMethod]
        public void TestString()
        {
            dynamic str = ROOTNET.Utility.ROOTCreator.CreateByName("string", "hi");
            Assert.AreEqual(2, str.size(), "Size of string");
            Assert.AreEqual("hi", str.c_str(), "String contents");
        }

        [TestMethod]
        public void TestVectorString()
        {
            dynamic vec = ROOTNET.Utility.ROOTCreator.CreateByName("vector<string,allocator<string> >");
            Assert.AreEqual((uint)0, vec.size(), "Size");
            dynamic str = ROOTNET.Utility.ROOTCreator.CreateByName("string", "hi");
            vec.push_back(str);
            Assert.AreEqual((uint)1, vec.size(), "size after push_back");
        }
    }
}
